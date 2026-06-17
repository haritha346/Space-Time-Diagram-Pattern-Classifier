#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

typedef struct {
    int min_r, max_r, min_c, max_c;
    int pixel_count;
} Shape;

// Logic that prioritizes solid blocks over individual lines
const char* identify_geometry(Shape s, int *type_idx) {
    int w = s.max_c - s.min_c + 1;
    int h = s.max_r - s.min_r + 1;
    float area = (float)w * h;
    float density = (float)s.pixel_count / area;
    float ratio = (float)w / h;

    // RULE 1: SQUARE & RECTANGLE (Priority)
    // If it's a solid cluster, it's a block, NOT a collection of lines.
    if (density > 0.60 && w >= 3 && h >= 3) {
        if (ratio >= 0.7 && ratio <= 1.3) { 
            *type_idx = 1; return "Square"; 
        } else { 
            *type_idx = 2; return "Rectangle"; 
        }
    }

    // RULE 2: STRAIGHT LINES
    // If it didn't pass Rule 1 and is very thin, it's a line.
    if (w <= 2 || h <= 2) {
        *type_idx = 0; return "Straight Line";
    }

    // RULE 3: TRIANGLES
    if (density >= 0.35 && density <= 0.60) {
        *type_idx = 3; return "Triangle";
    }

    // RULE 4: L and T SHAPES
    if (density > 0.50 && density <= 0.80) {
        if (w > h) { *type_idx = 4; return "T-Shape"; }
        else { *type_idx = 5; return "L-Shape"; }
    }

    *type_idx = -1;
    return "Unknown";
}

// Memory-safe Stack Flood Fill (Handles high-resolution grids on MacBook)
void find_blob(int r, int c, int rows, int cols, int **grid, int **visited, int target, Shape *s) {
    int *stack_r = malloc(rows * cols * sizeof(int));
    int *stack_c = malloc(rows * cols * sizeof(int));
    int top = 0;

    stack_r[top] = r; stack_c[top] = c;
    visited[r][c] = 1; top++;

    while (top > 0) {
        top--;
        int cr = stack_r[top]; int cc = stack_c[top];
        s->pixel_count++;

        if (cr < s->min_r) s->min_r = cr;
        if (cr > s->max_r) s->max_r = cr;
        if (cc < s->min_c) s->min_c = cc;
        if (cc > s->max_c) s->max_c = cc;

        int dr[] = {-1, 1, 0, 0};
        int dc[] = {0, 0, -1, 1};

        for (int i = 0; i < 4; i++) {
            int nr = cr + dr[i]; int nc = cc + dc[i];
            if (nr >= 0 && nr < rows && nc >= 0 && nc < cols && !visited[nr][nc] && grid[nr][nc] == target) {
                visited[nr][nc] = 1;
                stack_r[top] = nr; stack_c[top] = nc;
                top++;
            }
        }
    }
    free(stack_r); free(stack_c);
}

int main() {
    int width, height, channels;
    unsigned char *img = stbi_load("Wow.png", &width, &height, &channels, 0);
    if (!img) { printf("Error: SOS.png not found.\n"); return 1; }

    int rows, cols;
    printf("Enter number of rows: "); scanf("%d", &rows);
    printf("Enter number of columns: "); scanf("%d", &cols);

    int **grid = malloc(rows * sizeof(int *));
    int **visited = malloc(rows * sizeof(int *));
    for (int i = 0; i < rows; i++) {
        grid[i] = malloc(cols * sizeof(int));
        visited[i] = calloc(cols, sizeof(int));
    }

    // Binary Conversion & Save Output
    FILE *fp = fopen("output.txt", "w");
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int cx = (int)((c + 0.5) * width / cols);
            int cy = (int)((r + 0.5) * height / rows);
            int idx = (cy * width + cx) * channels;
            int gray = (img[idx] + img[idx+1] + img[idx+2]) / 3;
            grid[r][c] = (gray < 128) ? 1 : 0;
            fprintf(fp, "%d", grid[r][c]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);

    // DUAL MODE (Black and White)
    int targets[2] = {1, 0};
    const char* titles[2] = {"BLACK (1s)", "WHITE (0s)"};

    for (int m = 0; m < 2; m++) {
        int counts[6] = {0}; // Line, Sq, Rect, Tri, T, L
        for (int i = 0; i < rows; i++) for (int j = 0; j < cols; j++) visited[i][j] = 0;

        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                if (grid[r][c] == targets[m] && !visited[r][c]) {
                    Shape s = {r, r, c, c, 0};
                    find_blob(r, c, rows, cols, grid, visited, targets[m], &s);
                    
                    if (s.pixel_count >= 3) {
                        int type = -1;
                        identify_geometry(s, &type);
                        if (type != -1) counts[type]++;
                    }
                }
            }
        }

        printf("\n--- TOTAL PATTERNS IN %s ---\n", titles[m]);
        printf(" STRAIGHT LINES : %d\n", counts[0]);
        printf(" SQUARES        : %d\n", counts[1]);
        printf(" RECTANGLES     : %d\n", counts[2]);
        printf(" TRIANGLES      : %d\n", counts[3]);
        printf(" T-SHAPES       : %d\n", counts[4]);
        printf(" L-SHAPES       : %d\n", counts[5]);
    }

    for (int i = 0; i < rows; i++) { free(grid[i]); free(visited[i]); }
    free(grid); free(visited); stbi_image_free(img);
    return 0;
}