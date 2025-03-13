#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#define BUFFER_SIZE 4096

// Structure to store word counts.
typedef struct WordEntry {
    char *word;
    int count;
    struct WordEntry *next;
} WordEntry;

// Structure to store information for each file.
typedef struct FileData {
    char *filename;
    WordEntry *words;
    int total_words;
    struct FileData *next;
} FileData;

FileData *files_head = NULL;

/* Function Prototypes */
void process_path(const char *path);
void process_file(const char *filename);
void traverse_directory(const char *dirname);
void add_word(WordEntry **head, const char *word);
WordEntry* find_word(WordEntry *head, const char *word);
char* process_token(char *token);
int is_valid_word(const char *word);
void to_lower_case(char *str);
void free_word_entries(WordEntry *head);
void free_file_data(FileData *head);

/* Main: Process each command-line argument */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file or directory>...\n", argv[0]);
        return 1;
    }
    
    // Process each argument (file or directory)
    for (int i = 1; i < argc; i++) {
        process_path(argv[i]);
    }
    
    // Build the overall frequency table.
    WordEntry *overall = NULL;
    int overall_total = 0;
    for (FileData *f = files_head; f != NULL; f = f->next) {
        overall_total += f->total_words;
        for (WordEntry *w = f->words; w != NULL; w = w->next) {
            WordEntry *o = find_word(overall, w->word);
            if (o)
                o->count += w->count;
            else {
                o = malloc(sizeof(WordEntry));
                o->word = strdup(w->word);
                o->count = w->count;
                o->next = overall;
                overall = o;
            }
        }
    }
    
    // For each file, determine the word with the highest relative increase.
    for (FileData *f = files_head; f != NULL; f = f->next) {
        char *max_word = NULL;
        double max_ratio = 0.0;
        for (WordEntry *w = f->words; w != NULL; w = w->next) {
            WordEntry *o = find_word(overall, w->word);
            if (o) {
                double file_freq = (double) w->count / f->total_words;
                double overall_freq = (double) o->count / overall_total;
                double ratio = file_freq / overall_freq;
                if (ratio > max_ratio || (ratio == max_ratio && 
                    (max_word == NULL || strcmp(w->word, max_word) < 0))) {
                    max_ratio = ratio;
                    max_word = w->word;
                }
            }
        }
        if (max_word) {
            printf("%s: %s\n", f->filename, max_word);
        }
    }
    
    // Free allocated memory.
    free_word_entries(overall);
    free_file_data(files_head);
    
    return 0;
}

/* Process a given path, determining whether it is a file or directory */
void process_path(const char *path) {
    struct stat path_stat;
    if (stat(path, &path_stat) == -1) {
        perror(path);
        return;
    }
    if (S_ISDIR(path_stat.st_mode)) {
        traverse_directory(path);
    } else if (S_ISREG(path_stat.st_mode)) {
        process_file(path);
    }
}

/* Process a single file */
void process_file(const char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror(filename);
        return;
    }
    
    // Create a new FileData structure for this file.
    FileData *fileData = malloc(sizeof(FileData));
    fileData->filename = strdup(filename);
    fileData->words = NULL;
    fileData->total_words = 0;
    fileData->next = files_head;
    files_head = fileData;
    
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    char token[BUFFER_SIZE];
    int token_index = 0;
    
    // Read the file chunk by chunk.
    while ((bytes_read = read(fd, buffer, BUFFER_SIZE)) > 0) {
        for (ssize_t i = 0; i < bytes_read; i++) {
            if (!isspace(buffer[i])) {
                token[token_index++] = buffer[i];
            } else {
                if (token_index > 0) {
                    token[token_index] = '\0';
                    char *word = process_token(token);
                    if (word != NULL && is_valid_word(word)) {
                        add_word(&fileData->words, word);
                        fileData->total_words++;
                        free(word); // free temporary allocation from process_token
                    }
                    token_index = 0;
                }
            }
        }
    }
    // Process any token remaining in the buffer.
    if (token_index > 0) {
        token[token_index] = '\0';
        char *word = process_token(token);
        if (word != NULL && is_valid_word(word)) {
            add_word(&fileData->words, word);
            fileData->total_words++;
            free(word);
        }
    }
    close(fd);
}

/* Recursively traverse a directory */
void traverse_directory(const char *dirname) {
    DIR *dir = opendir(dirname);
    if (!dir) {
        perror(dirname);
        return;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue; // ignore hidden files/directories
        
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dirname, entry->d_name);
        struct stat path_stat;
        if (stat(path, &path_stat) == -1) {
            perror(path);
            continue;
        }
        if (S_ISDIR(path_stat.st_mode)) {
            traverse_directory(path);
        } else if (S_ISREG(path_stat.st_mode)) {
            // For directories, only process files ending with ".txt"
            char *ext = strrchr(entry->d_name, '.');
            if (ext && strcmp(ext, ".txt") == 0) {
                process_file(path);
            }
        }
    }
    closedir(dir);
}

/* Add a word to the word list or increment its count if already present */
void add_word(WordEntry **head, const char *word) {
    WordEntry *entry = find_word(*head, word);
    if (entry) {
        entry->count++;
    } else {
        entry = malloc(sizeof(WordEntry));
        entry->word = strdup(word);
        entry->count = 1;
        entry->next = *head;
        *head = entry;
    }
}

/* Find a word in the list */
WordEntry* find_word(WordEntry *head, const char *word) {
    for (; head != NULL; head = head->next) {
        if (strcmp(head->word, word) == 0)
            return head;
    }
    return NULL;
}

/* Process a token: trim disallowed leading and trailing punctuation and convert to lower-case.
   Returns a newly allocated string (or NULL if the token is not valid). */
char* process_token(char *token) {
    char *start = token;
    // Remove leading punctuation characters if present.
    while (*start && (*start == '(' || *start == '[' || *start == '{' || *start == '"' || *start == '\'')) {
        start++;
    }
    // Remove trailing punctuation.
    char *end = start + strlen(start) - 1;
    while (end > start && (*end == ')' || *end == ']' || *end == '}' || *end == '"' ||
           *end == '\'' || *end == ',' || *end == '.' || *end == '!' || *end == '?')) {
        *end = '\0';
        end--;
    }
    
    // Ensure the token has at least one letter.
    int has_letter = 0;
    for (char *p = start; *p; p++) {
        if (isalpha(*p)) { has_letter = 1; break; }
    }
    if (!has_letter) return NULL;
    
    char *result = strdup(start);
    to_lower_case(result);
    return result;
}

/* Check if a processed word is valid (non-empty) */
int is_valid_word(const char *word) {
    return (word && strlen(word) > 0);
}

/* Convert a string to lower case */
void to_lower_case(char *str) {
    for (; *str; str++) {
        *str = tolower(*str);
    }
}

/* Free a linked list of word entries */
void free_word_entries(WordEntry *head) {
    while (head) {
        WordEntry *temp = head;
        head = head->next;
        free(temp->word);
        free(temp);
    }
}

/* Free the FileData list */
void free_file_data(FileData *head) {
    while (head) {
        FileData *temp = head;
        head = head->next;
        free(temp->filename);
        free_word_entries(temp->words);
        free(temp);
    }
}