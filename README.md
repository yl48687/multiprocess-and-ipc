# Multiprocess and IPC
This project contains C code implementing a word counting program using multiprocesses and inter-process communication (IPC). The program utilizes multiple child processes to count the number of lines, words, and characters in a given text file concurrently, achieving parallelism through process creation and communication via pipes.

## Design Overview
The project architecture revolves around creating multiple child processes to distribute the workload of counting words in a file. Each child process is responsible for reading a portion of the file, counting words, and communicating the results back to the parent process using pipes. The parent process coordinates the creation and termination of child processes, as well as aggregating the word counts from each child.

## Functionality
`wc_core.c`:
- Implements the `word_count` function, which reads a portion of a file specified by an offset and size, `wc_multi.c`:
- Contains the `main` function implementing the multiprocess word counting program.
- Parses command-line arguments specifying the filename, number of child processes, and crash rate.
- Creates child processes to distribute the workload of counting words in the file.
- Utilizes pipes for inter-process communication, allowing child processes to send their word count results back to the parent process.
- Aggregates word counts from all child processes and prints the total number of lines, words, and characters in the file.

## File Structure and Content
```
multiprocess-and-ipc/
├── Makefile
├── README.md
├── Report.pdf
├── resources/
│   ├── large.txt
│   └── small.txt
├── wc_core.c
├── wc.h
└── wc_multi.c
```

For more detailed information about this project, refer to the "Report.pdf" included in the repository.