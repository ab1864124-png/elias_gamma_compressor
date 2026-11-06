#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define MAX_WORD_LEN 255
/* --- Bit Stream Functions --- */
void write_bit(FILE *fp, unsigned char *byte, int *bit_pos, int bit) {
    *byte = (unsigned char)((*byte << 1) | (bit & 1));
    (*bit_pos)++;
    if (*bit_pos == 8) {
        fputc(*byte, fp);
        *byte = 0;
        *bit_pos = 0;
    }
}
void flush_bits(FILE *fp, unsigned char *byte, int *bit_pos) {
    if (*bit_pos > 0) {
        *byte <<= (8 - *bit_pos);
        fputc(*byte, fp);
        *byte = 0;
        *bit_pos = 0;
    }
}
int read_bit(FILE *fp, unsigned char *byte, int *bits_left) {
    if (*bits_left == 0) {
        int c = fgetc(fp);
        if (c == EOF) return -1;
        *byte = (unsigned char)c;
        *bits_left = 8;
    }
    int bit = (*byte >> (*bits_left - 1)) & 1;
    (*bits_left)--;
    return bit;
}
/* --- Elias Gamma Encoding & Decoding --- */
void encode_gamma(FILE *fp, unsigned char *byte, int *bit_pos, int rank) {
    int val = rank + 1;
    int len = 0;
    for (int tmp = val; tmp > 1; tmp >>= 1) len++;
    for (int i = 0; i < len; i++) write_bit(fp, byte, bit_pos, 0);
    for (int i = len; i >= 0; i--) write_bit(fp, byte, bit_pos, (val >> i) & 1);
}
int decode_gamma(FILE *fp, unsigned char *byte, int *bits_left) {
    int len = 0, bit;
    while ((bit = read_bit(fp, byte, bits_left)) == 0) len++;
    if (bit == -1) return 0;
    int val = 1;
    for (int i = 0; i < len; i++) val = (val << 1) | read_bit(fp, byte, bits_left);
    return val - 1;
}
/* --- Dictionary Lookup using Parallel Arrays --- */
int find_or_insert(char ***words, int **freqs, int **ids, int *size, int *cap, const char *word) {
    for (int i = 0; i < *size; i++) {
        if (strcmp((*words)[i], word) == 0) {
            (*freqs)[i]++;
            return i;
        }
    }
    if (*size >= *cap) {
        *cap *= 2;
        *words = (char **)realloc(*words, (*cap) * sizeof(char *));
        *freqs = (int *)realloc(*freqs, (*cap) * sizeof(int));
        *ids = (int *)realloc(*ids, (*cap) * sizeof(int));
    }
    int idx = (*size)++;
    (*words)[idx] = strdup(word);
    (*freqs)[idx] = 1;
    (*ids)[idx] = idx;
    return idx;
}
/* --- 1. Compress input.txt -> compressed.dat --- */
int compress(const char *input_path, const char *compressed_path) {
    FILE *in = fopen(input_path, "rb");
    if (!in) { perror("Error opening input file"); return -1; }
    int dict_size = 0, dict_cap = 256;
    char **dict_words = (char **)malloc(dict_cap * sizeof(char *));
    int *dict_freqs = (int *)malloc(dict_cap * sizeof(int));
    int *dict_ids = (int *)malloc(dict_cap * sizeof(int));
    int tok_count = 0, tok_cap = 1024;
    int *tokens = (int *)malloc(tok_cap * sizeof(int));
    int c = fgetc(in);
    while (c != EOF) {
        char buf[MAX_WORD_LEN + 1];
        int len = 0;
        int is_word = isalnum(c) || c == '_';
        while (c != EOF && (isalnum(c) || c == '_') == is_word && len < MAX_WORD_LEN) {
            buf[len++] = (char)c;
            c = fgetc(in);
        }
        buf[len] = '\0';
        int id = find_or_insert(&dict_words, &dict_freqs, &dict_ids, &dict_size, &dict_cap, buf);
        if (tok_count >= tok_cap) {
            tok_cap *= 2;
            tokens = (int *)realloc(tokens, tok_cap * sizeof(int));
        }
        tokens[tok_count++] = id;
        if (is_word && len < MAX_WORD_LEN && c == ' ') {
            int next = fgetc(in);
            if (next != EOF && (isalnum(next) || next == '_')) {
                c = next;
            } else if (next != EOF) {
                ungetc(next, in);
            }
        }
    }
    fclose(in);
    /* Simple bubble sort on parallel arrays by frequency (descending) */
    for (int i = 0; i < dict_size - 1; i++) {
        for (int j = 0; j < dict_size - i - 1; j++) {
            if (dict_freqs[j] < dict_freqs[j + 1]) {
                int temp_f = dict_freqs[j]; dict_freqs[j] = dict_freqs[j + 1]; dict_freqs[j + 1] = temp_f;
                int temp_id = dict_ids[j]; dict_ids[j] = dict_ids[j + 1]; dict_ids[j + 1] = temp_id;
                char *temp_w = dict_words[j]; dict_words[j] = dict_words[j + 1]; dict_words[j + 1] = temp_w;
            }
        }
    }
    int *rank_map = (int *)malloc((dict_size ? dict_size : 1) * sizeof(int));
    for (int rank = 0; rank < dict_size; rank++) rank_map[dict_ids[rank]] = rank;
    FILE *out = fopen(compressed_path, "wb");
    if (!out) {
        perror("Error opening compressed file");
        free(rank_map); free(tokens); free(dict_freqs); free(dict_ids);
        for (int i = 0; i < dict_size; i++) free(dict_words[i]);
        free(dict_words);
        return -1;
    }
    fwrite(&dict_size, sizeof(int), 1, out);
    fwrite(&tok_count, sizeof(int), 1, out);
    for (int i = 0; i < dict_size; i++) {
        unsigned char len = (unsigned char)strlen(dict_words[i]);
        fwrite(&len, sizeof(unsigned char), 1, out);
        fwrite(dict_words[i], 1, len, out);
    }
    unsigned char write_byte = 0;
    int write_pos = 0;
    for (int i = 0; i < tok_count; i++) encode_gamma(out, &write_byte, &write_pos, rank_map[tokens[i]]);
    flush_bits(out, &write_byte, &write_pos);
    fclose(out);
    free(rank_map); free(tokens); free(dict_freqs); free(dict_ids);
    for (int i = 0; i < dict_size; i++) free(dict_words[i]);
    free(dict_words);
    return 0;
}
/* --- 2. Decompress compressed.dat -> decompressed.txt --- */
int decompress(const char *compressed_path, const char *output_path) {
    FILE *in = fopen(compressed_path, "rb");
    if (!in) { perror("Error opening compressed file"); return -1; }
    int dict_size = 0, tok_count = 0;
    if (fread(&dict_size, sizeof(int), 1, in) != 1 || fread(&tok_count, sizeof(int), 1, in) != 1) {
        fprintf(stderr, "Error reading header.\n");
        fclose(in);
        return -1;
    }
    char **dict_words = (char **)malloc((dict_size ? dict_size : 1) * sizeof(char *));
    for (int i = 0; i < dict_size; i++) {
        unsigned char len = 0;
        if (fread(&len, sizeof(unsigned char), 1, in) != 1) { fclose(in); return -1; }
        dict_words[i] = (char *)malloc(len + 1);
        if (fread(dict_words[i], 1, len, in) != len) { fclose(in); return -1; }
        dict_words[i][len] = '\0';
    }
    FILE *out = fopen(output_path, "wb");
    if (!out) {
        perror("Error opening output file");
        for (int i = 0; i < dict_size; i++) free(dict_words[i]);
        free(dict_words); fclose(in);
        return -1;
    }
    unsigned char read_byte = 0;
    int bits_left = 0;
    int prev_was_word = 0;
    size_t prev_len = 0;
    for (int i = 0; i < tok_count; i++) {
        int rank = decode_gamma(in, &read_byte, &bits_left);
        if (rank >= dict_size) { fprintf(stderr, "Corrupted bitstream.\n"); break; }
        const char *word = dict_words[rank];
        int curr_is_word = isalnum((unsigned char)word[0]) || word[0] == '_';
        if (prev_was_word && curr_is_word && prev_len < MAX_WORD_LEN) fputc(' ', out);
        fputs(word, out);
        prev_was_word = curr_is_word;
        prev_len = strlen(word);
    }
    fclose(out);
    fclose(in);
    for (int i = 0; i < dict_size; i++) free(dict_words[i]);
    free(dict_words);
    return 0;
}
int main(void) {
    if (compress("input.txt", "compressed.dat") != 0) return EXIT_FAILURE;
    printf("Compression complete: input.txt -> compressed.dat\n");
    if (decompress("compressed.dat", "decompressed.txt") != 0) return EXIT_FAILURE;
    printf("Decompression complete: compressed.dat -> decompressed.txt\n");
    return EXIT_SUCCESS;
}