
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>

int main() {
    char studentID[50] = "BC240417219";
    int times[3];
    int total, maxTime, longestTask;
    int pipe1[2], pipe2[2];
    pid_t pid;

    printf("Enter Student ID: %s\n", studentID);

    for (int i = 0; i < 3; i++) {
        do {
            printf("Enter Task %d Execution Time: ", i + 1);
            scanf("%d", &times[i]);

            if (times[i] <= 0)
                printf("Please enter a positive integer.\n");

        } while (times[i] <= 0);
    }

    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("Pipe creation failed");
        return 1;
    }

    pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        return 1;
    }

    if (pid == 0) {
        // Child Process
        close(pipe1[1]);
        close(pipe2[0]);

        int received[3];

        read(pipe1[0], received, sizeof(received));

        total = received[0] + received[1] + received[2];

        maxTime = received[0];
        longestTask = 1;

        for (int i = 1; i < 3; i++) {
            if (received[i] > maxTime) {
                maxTime = received[i];
                longestTask = i + 1;
            }
        }

        int result[3] = {total, longestTask, maxTime};

        write(pipe2[1], result, sizeof(result));

        close(pipe1[0]);
        close(pipe2[1]);

        exit(0);
    }
    else {
        // Parent Process
        close(pipe1[0]);
        close(pipe2[1]);

        write(pipe1[1], times, sizeof(times));

        int result[3];

        read(pipe2[0], result, sizeof(result));

        waitpid(pid, NULL, 0);

        close(pipe1[1]);
        close(pipe2[0]);

        total = result[0];
        longestTask = result[1];
        maxTime = result[2];

        char report[1000];

        snprintf(report, sizeof(report),
            "\n--- Task Processing Report ---\n"
            "Student ID: %s\n"
            "Task 1: %d units\n"
            "Task 2: %d units\n"
            "Task 3: %d units\n"
            "Total Execution Time: %d units\n"
            "Longest Task: Task %d\n"
            "Maximum Execution Time: %d units\n",
            studentID,
            times[0], times[1], times[2],
            total, longestTask, maxTime);

        printf("%s", report);

        char filename[100];
        snprintf(filename, sizeof(filename),
                 "%s_report.txt", studentID);

        int fd = open(filename,
                      O_WRONLY | O_CREAT | O_TRUNC,
                      0644);

        if (fd < 0) {
            perror("File opening failed");
            return 1;
        }

        write(fd, report, strlen(report));
        close(fd);

        printf("Report saved to: %s\n", filename);
    }

    return 0;
}
