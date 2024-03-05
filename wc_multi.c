#include "wc.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

int main(int argc, char **argv) {
    long fsize;
    FILE *fp;
    count_t count;
    struct timespec begin, end;
    int nChildProc = 0;

    /* 1st arg: filename */
    if (argc < 2) {
        printf("usage: wc <filename> [# processes] [crash rate]\n");
        return 0;
    } // if

    /* 2nd (optional) arg: number of child processes */
    if (argc > 2) {
        nChildProc = atoi(argv[2]);
        if (nChildProc < 1) nChildProc = 1;
        if (nChildProc > 10) nChildProc = 10;
    } // if

    /* 3rd (optional) arg: crash rate between 0% and 100%. Each child process has that much chance to crash */
    if (argc > 3) {
        crashRate = atoi(argv[3]);
        if (crashRate < 0) crashRate = 0;
        if (crashRate > 50) crashRate = 50;
    } // if

    printf("# of Child Processes: %d\n", nChildProc);
    printf("crashRate RATE: %d%%\n", crashRate);

    count.linecount = 0;
    count.wordcount = 0;
    count.charcount = 0;

    // Measure the start time
    clock_gettime(CLOCK_REALTIME, &begin);

    // Open file in read-only mode
    fp = fopen(argv[1], "r");

    if (fp == NULL) {
        printf("File open error: %s\n", argv[1]);
        printf("usage: wc <filename>\n");
        return 0;
    } // if

    // Get a file size
    fseek(fp, 0L, SEEK_END);
    fsize = ftell(fp);

    // Reset file position indicator to the beginning of the file
    fseek(fp, 0L, SEEK_SET);

    // Create pipes for communication
    int pipes[nChildProc][2];

    // Create an array to store child process PIDs
    pid_t child_pids[nChildProc];

    // Create child processes
    for (int i = 0; i < nChildProc; ++i) {
        if (pipe(pipes[i]) == -1) {
            perror("Pipe creation failed.");
            exit(EXIT_FAILURE);
        } // if

		// Fork
        pid_t child_pid = fork();

        if (child_pid == -1) {
            perror("Fork failed.");
            exit(EXIT_FAILURE);
        } // if

		// Child process
        if (child_pid == 0) {
            close(pipes[i][0]);

            // Calculate offset and size for the child process
            long offset = i * (fsize / nChildProc);
            long size = (i == nChildProc - 1) ? (fsize - offset) : (fsize / nChildProc);

            // Open the file in the child process
            FILE* child_fp = fopen(argv[1], "r");
            if (child_fp == NULL) {
                perror("Child process - File open error.");
                exit(EXIT_FAILURE);
            } // if

            // Move file pointer to the correct position
            if (fseek(child_fp, offset, SEEK_SET) < 0) {
                perror("Child process - fseek error.");
                exit(EXIT_FAILURE);
            } // if

            // Adjust size for the last process to read until the end of the file
            if (i == nChildProc - 1) {
                size = fsize - offset;
            } // if

            count = word_count(child_fp, offset, size);

            // Write count results to the pipe
            write(pipes[i][1], &count, sizeof(count));

            close(pipes[i][1]);
            fclose(child_fp);
            exit(EXIT_SUCCESS);
        } else {
            child_pids[i] = child_pid;
        } // if
    } // for

    // Parent process
    fclose(fp);

    // Create variables to accumulate counts
    int totalLines = 0, totalWords = 0, totalChars = 0;

    // Wait for child processes to complete in order
    for (int i = 0; i < nChildProc; ++i) {
        int status;
        waitpid(child_pids[i], &status, 0);

		// Child process exited successfully
        if (WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS) {
            // Read from pipes if needed
            read(pipes[i][0], &count, sizeof(count));
            // Accumulate counts
            totalLines += count.linecount;
            totalWords += count.wordcount;
            totalChars += count.charcount;
        } // if
        while (WIFSIGNALED(status)) {
            // Restart the crashed child process
            pid_t new_pid = fork();

            if (new_pid == -1) {
                perror("Fork failed.");
                exit(EXIT_FAILURE);
            } // if

			// New child process
            if (new_pid == 0) {
                close(pipes[i][0]);

                // Calculate offset and size for the child process
                long offset = i * (fsize / nChildProc);
                long size = (i == nChildProc - 1) ? (fsize - offset) : (fsize / nChildProc);

                // Open the file in the child process
                FILE* child_fp = fopen(argv[1], "r");
                if (child_fp == NULL) {
                    perror("Child process - File open error.");
                    exit(EXIT_FAILURE);
                } // if

                // Move file pointer to the correct position
                if (fseek(child_fp, offset, SEEK_SET) < 0) {
                    perror("Child process - fseek error.");
                    exit(EXIT_FAILURE);
                } // if

                // Adjust size for the last process to read until the end of the file
                if (i == nChildProc - 1) {
                    size = fsize - offset;
                } // if

                count = word_count(child_fp, offset, size);

                // Write count results to the pipe
                write(pipes[i][1], &count, sizeof(count));

                close(pipes[i][1]);  // Close write end of the pipe after writing
                fclose(child_fp);    // Close file in child process
                exit(EXIT_SUCCESS);
            } else {
                // Wait for the newly forked child process
                waitpid(new_pid, &status, 0);

                if (WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS) {
                    // Read from pipes if needed
                    read(pipes[i][0], &count, sizeof(count));

                    // Accumulate counts
                    totalLines += count.linecount;
                    totalWords += count.wordcount;
                    totalChars += count.charcount;
                } // if
            } // if
        } // while
    } // for

    // Assign accumulated counts to count structure
    count.linecount = totalLines;
    count.wordcount = totalWords;
    count.charcount = totalChars;

    // Measure the end time
    clock_gettime(CLOCK_REALTIME, &end);
    long seconds = end.tv_sec - begin.tv_sec;
    long nanoseconds = end.tv_nsec - begin.tv_nsec;
    double elapsed = seconds + nanoseconds * 1e-9;

	// Print statements
    printf("\n========= %s =========\n", argv[1]);
    printf("Total Lines : %d \n", count.linecount);
    printf("Total Words : %d \n", count.wordcount);
    printf("Total Characters : %d \n", count.charcount);
    printf("======== Took %.3f seconds ========\n", elapsed);

    return 0;
} // main
