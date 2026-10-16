// week5_task3_student_management_system.c
// Name: ________  Surname: ________  Student ID: ________
//
// Task 3: Mini-project – Student management system with file persistence
// Week 5 – Files & Modular Programming
// TODO: Implement the functions to load, save, add, and list students.
//
// Build: make lab5   Run: ./bin/week5_task3_student_management_system

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STUDENTS 100
#define NAME_LEN 50
#define LINE_LEN 256
#define DATA_FILE "students.txt"

typedef struct Student {
    char name[NAME_LEN];
    int id;
    float gpa;
} Student;

// Function prototypes (do not change these signatures – the autograder calls them directly)
int  load_students(Student arr[], int max);          // returns number of records loaded
int  save_students(const Student arr[], int count);  // 0 on success, -1 on error
void add_student(Student arr[], int *count);
void list_students(const Student arr[], int count);
int  read_line(const char *prompt, char *buf, size_t size);

int main(void) {
    Student students[MAX_STUDENTS];
    int count = 0;
    char input[LINE_LEN];
    int running = 1;

    // TODO: Load existing data: count = load_students(students, MAX_STUDENTS);
    count = load_students(students, MAX_STUDENTS);
    printf("Loaded %d student(s) from %s\n", count, DATA_FILE);

    while (running) {
        printf("\n=== Student Management System ===\n");
        printf("1. List students\n");
        printf("2. Add student\n");
        printf("3. Save and Exit\n");

        int choice;
        if (read_line("Select an option: ", input, sizeof input) != 0) {
            choice = 3;                         // end of input: save and exit instead of looping forever
        } else {
            char *end;
            long value = strtol(input, &end, 10);
            choice = (end != input && *end == '\0') ? (int)value : 0;   // non-numbers become invalid
        }

        switch (choice) {
            case 1:
                // TODO: Call list_students()
                list_students(students, count);
                break;
            case 2:
                // TODO: Call add_student()
                add_student(students, &count);
                break;
            case 3:
                // TODO: Call save_students(). If it fails, print an error message.
                if (save_students(students, count) == -1) {
                    perror("Error saving data");
                }
                printf("Data saved. Goodbye!\n");
                running = 0;
                break;
            default:
                printf("Invalid option. Try again.\n");
        }
    }

    return EXIT_SUCCESS;
}

// Provided helper – prints a prompt and reads one line from stdin into buf.
// The trailing newline is removed. Returns 0 on success, -1 on end of input (EOF).
int read_line(const char *prompt, char *buf, size_t size) {
    printf("%s", prompt);
    fflush(stdout);
    if (fgets(buf, (int)size, stdin) == NULL) {
        return -1;
    }
    size_t len = strcspn(buf, "\r\n");
    if (buf[len] == '\0' && len == size - 1) {
        // Line was longer than the buffer: discard the rest of it.
        int c;
        while ((c = getchar()) != '\n' && c != EOF) { }
    }
    buf[len] = '\0';
    return 0;
}

// TODO: Implement load_students()
// Open DATA_FILE for reading. If it cannot be opened (e.g. first run), return 0.
// Read the file line by line with fgets(); parse "name,id,gpa" with
// sscanf(line, "%49[^,],%d,%f", ...) and keep only lines where sscanf returned 3.
// Stop when max records have been loaded. Close the file and return the number loaded.
int load_students(Student arr[], int max) {
    FILE *fp = fopen(DATA_FILE, "r");
    if (fp == NULL) {
        return 0;
    }

    char line[LINE_LEN];
    int loaded_count = 0;

    while (loaded_count < max && fgets(line, sizeof(line), fp) != NULL) {
        char temp_name[NAME_LEN];
        int temp_id;
        float temp_gpa;

        if (sscanf(line, "%49[^,],%d,%f", temp_name, &temp_id, &temp_gpa) == 3) {
            strcpy(arr[loaded_count].name, temp_name);
            arr[loaded_count].id = temp_id;
            arr[loaded_count].gpa = temp_gpa;
            loaded_count++;
        }
    }

    fclose(fp);
    return loaded_count;
}

// TODO: Implement save_students()
// Open DATA_FILE for writing ("w"), return -1 if fopen() fails.
// Write one line per student: fprintf(fp, "%s,%d,%.2f\n", ...). Close the file and return 0.
int save_students(const Student arr[], int count) {
    FILE *fp = fopen(DATA_FILE, "w");
    if (fp == NULL) {
        return -1;
    }

    for (int i = 0; i < count; i++) {
        fprintf(fp, "%s,%d,%.2f\n", arr[i].name, arr[i].id, arr[i].gpa);
    }

    fclose(fp);
    return 0;
}

// TODO: Implement add_student()
// 1. If *count >= MAX_STUDENTS, print "Cannot add more students (limit reached)." and return.
// 2. Use read_line() to ask "Enter name: ", "Enter ID: " and "Enter GPA: ".
//    If read_line() returns -1 (end of input), just return.
// 3. Validate: name not empty, no ',' in name, ID is an integer, GPA is a number in 0.0–4.0
//    (strtol / strtof are useful). Otherwise print "Invalid input. Student not added." and return.
// 4. Store the student at arr[*count], increase *count, print "Student added successfully!"
void add_student(Student arr[], int *count) {
    if (*count >= MAX_STUDENTS) {
        printf("Cannot add more students (limit reached).\n");
        return;
    }

    char name_buf[LINE_LEN];
    char id_buf[LINE_LEN];
    char gpa_buf[LINE_LEN];

    if (read_line("Enter name: ", name_buf, sizeof(name_buf)) == -1) return;
    if (read_line("Enter ID: ", id_buf, sizeof(id_buf)) == -1) return;
    if (read_line("Enter GPA: ", gpa_buf, sizeof(gpa_buf)) == -1) return;

    // Validate Name: not empty and no commas
    if (strlen(name_buf) == 0 || strchr(name_buf, ',') != NULL) {
        printf("Invalid input. Student not added.\n");
        return;
    }

    // Validate ID: must be a valid integer
    char *endptr;
    long id_val = strtol(id_buf, &endptr, 10);
    if (endptr == id_buf || *endptr != '\0') {
        printf("Invalid input. Student not added.\n");
        return;
    }

    // Validate GPA: must be a float between 0.0 and 4.0
    float gpa_val = strtof(gpa_buf, &endptr);
    if (endptr == gpa_buf || *endptr != '\0' || gpa_val < 0.0f || gpa_val > 4.0f) {
        printf("Invalid input. Student not added.\n");
        return;
    }

    // Add student
    strcpy(arr[*count].name, name_buf);
    arr[*count].id = (int)id_val;
    arr[*count].gpa = gpa_val;
    (*count)++;

    printf("Student added successfully!\n");
}

// TODO: Implement list_students()
// If count is 0 print "No students found."; otherwise print a header and one line per student,
// e.g. printf("%-5d %-20s %.2f\n", id, name, gpa);
void list_students(const Student arr[], int count) {
    if (count == 0) {
        printf("No students found.\n");
        return;
    }

    printf("%-5s %-20s %s\n", "ID", "Name", "GPA");
    for (int i = 0; i < count; i++) {
        printf("%-5d %-20s %.2f\n", arr[i].id, arr[i].name, arr[i].gpa);
    }
}
