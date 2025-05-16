# MicroGit C++ Version

This is the C++ implementation of MicroGit, a lightweight version control system designed to demonstrate the fundamentals of version control.

## Building the Project

### Prerequisites

- C++17 compatible compiler
- CMake (version 3.10 or higher)
- OpenSSL development libraries

### Build Instructions

```bash
mkdir build
cd build
cmake ..
make
```

Alternatively, you can use the provided build script:

```bash
./build.sh
```

This will create the `microgit` executable in the build directory.

## Usage

Run `./microgit --help` to see available commands.

### Implemented Commands

#### Initialize a Repository

```bash
./microgit init
```

Creates a new MicroGit repository in the current directory.

#### Add Files

```bash
./microgit add <file1> [file2] [file3] ...
```

Adds the specified files to the staging area.

#### Save Changes (Commit)

```bash
./microgit save "Commit message"
```

or interactively:

```bash
./microgit save
# Enter commit message when prompted
```

Saves all staged changes to the repository.

#### View Commit History

```bash
./microgit log
```

or with a limit:

```bash
./microgit log 5  # Show only the last 5 commits
```

Displays the commit history.

#### Check Status

```bash
./microgit status
```

Shows the current status of the working directory, including:

- Changes to be committed (staged)
- Changes not staged for commit
- Untracked files

#### Checkout Files or Commits

```bash
./microgit checkout <commit-hash>  # Checkout entire commit
./microgit checkout <commit-hash> <filename>  # Checkout specific file from commit
./microgit checkout <filename>  # Checkout file from HEAD
```

Restores files from a specific commit or the current HEAD.

#### Remove from Staging

```bash
./microgit remove <file1> [file2] [file3] ...
```

Removes files from the staging area (unstaging them).

## Repository Structure

MicroGit creates a `.microgit` directory with the following structure:

```
.microgit/
  ├── HEAD        # References the current commit
  ├── objects/    # Stores all file content and commits
  └── staging/    # Staging area for files to be committed
```

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.
