#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>   
#include <stdio.h>       
#include <stdlib.h>      
#include <string.h>      
#include <stdbool.h>     
#include <time.h>        

#define MAX_ROWS 1024
#define MAX_COLS 1024

int grid[MAX_ROWS][MAX_COLS];
int temp_binary[MAX_ROWS][MAX_COLS];
int binary_length[MAX_ROWS];
char stored_decimals[MAX_ROWS][512]; 

int actual_rows = 0;
int actual_cols = 0;

// Convert decimal string → binary
void process_decimal_line(char* decimalStr, int row_index) {
    int binary[MAX_COLS];
    int binaryIdx = 0;
    int len = strlen(decimalStr);
    int digits[512];

    for(int i = 0; i < len; i++) digits[i] = decimalStr[i] - '0';

    while (true) {
        int remainder = 0;
        bool allZero = true;

        for (int i = 0; i < len; i++) {
            int current = digits[i] + remainder * 10;
            digits[i] = current / 2;
            remainder = current % 2;
            if (digits[i] > 0) allZero = false;
        }

        binary[binaryIdx++] = remainder;

        if (allZero || binaryIdx >= MAX_COLS) break;
    }

    // Reverse
    for (int i = 0; i < binaryIdx; i++)
        temp_binary[row_index][i] = binary[binaryIdx - 1 - i];

    binary_length[row_index] = binaryIdx;
}

// Random decimal generator
void generate_random_decimal_string(char* str) {
    int digit_len = (rand() % 15) + 1;

    for (int i = 0; i < digit_len; i++) {
        if (i == 0) str[i] = (rand() % 9) + '1';
        else str[i] = (rand() % 10) + '0';
    }
    str[digit_len] = '\0';
}

void setup_initial_state() {
    srand(time(NULL));

    printf("Enter number of rows: ");
    scanf("%d", &actual_rows);

    if (actual_rows > MAX_ROWS) actual_rows = MAX_ROWS;

    printf("\n--- GENERATED DECIMAL NUMBERS ---\n");

    int max_len = 0;

    // Generate + convert
    for (int i = 0; i < actual_rows; i++) {
        generate_random_decimal_string(stored_decimals[i]);

        printf("Row %d: %s\n", i + 1, stored_decimals[i]);

        process_decimal_line(stored_decimals[i], i);

        // ✅ Find maximum binary length
        if (binary_length[i] > max_len)
            max_len = binary_length[i];
    }

    // ✅ Set columns automatically
    actual_cols = max_len;

    printf("\nAuto-detected column size (bits): %d\n", actual_cols);

    // Build grid with padding
    for (int r = 0; r < actual_rows; r++) {
        for (int c = 0; c < actual_cols; c++)
            grid[r][c] = 0;

        int len = binary_length[r];
        int start = actual_cols - len;

        for (int i = 0; i < len; i++)
            grid[r][start + i] = temp_binary[r][i];
    }

    // Save to file
    FILE *outputFile = fopen("output.txt", "w");

    if (outputFile) {
        fprintf(outputFile, "=== DECIMAL NUMBERS ===\n");
        for (int r = 0; r < actual_rows; r++)
            fprintf(outputFile, "R%03d: %s\n", r + 1, stored_decimals[r]);

        fprintf(outputFile, "\n=== BINARY GRID ===\n");
        for (int r = 0; r < actual_rows; r++) {
            fprintf(outputFile, "R%03d: ", r + 1);
            for (int c = 0; c < actual_cols; c++)
                fprintf(outputFile, "%d", grid[r][c]);
            fprintf(outputFile, "\n");
        }

        fclose(outputFile);
        printf("\nSaved to output.txt\n");
    }
}

// OpenGL display
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    float cell_w = 2.0f / actual_cols;
    float cell_h = 2.0f / actual_rows;

    for (int r = 0; r < actual_rows; r++) {
        for (int c = 0; c < actual_cols; c++) {

            if (grid[r][c] == 1)
                glColor3f(0, 0, 0);
            else
                glColor3f(1, 1, 1);

            float x = -1.0f + c * cell_w;
            float y = 1.0f - r * cell_h;

            glBegin(GL_QUADS);
                glVertex2f(x, y);
                glVertex2f(x + cell_w, y);
                glVertex2f(x + cell_w, y - cell_h);
                glVertex2f(x, y - cell_h);
            glEnd();
        }
    }

    glFlush();
}

int main(int argc, char** argv) {
    setup_initial_state();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(800, 800);
    glutCreateWindow("Auto Binary Grid");

    gluOrtho2D(-1, 1, -1, 1);

    glutDisplayFunc(display);
    glutMainLoop();

    return 0;
}