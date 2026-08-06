# File & Folder Version Management System

## Introduction

This project was previously called "File Version Management".

Usually people choose to backup a folder by copying the entire folder. In this case, a large number of files or folders that have not been modified are also backed up. In the overall view, the same unmodified file is saved several times, which causes waste of storage space. This system is designed to solve that problem.

The system ensures that each file with the same content is saved only once (even if the names are different). It is also possible to create a new version and have it inherit from a previous version. The new version can access all the content of the previous version, but the two versions were independent of each other. In other words, adding, deleting, or modifying files or folders in the new version will not have any effect on the version it inherits.

## Usage

The system is written in MacOS system. Make sure you have g++ or another compiler that can compile cpp files installed on your computer before using it. You can compile the `main.c` with g++ to generate an executable file. If you do not know this very well, then you can first execute `g++ main.c -o ffvms` to generate the executable file `ffvms`, and then execute `sudo ./ffvms` can start to use the system.

Please note that if your computer system is not MacOS, some of the features of this system may not work properly on your computer.

The system operates via the command line.

The following describes the commands that will be used in the system.

### Commands in the system

| Command ID | Command                | Note                                                         | Example                                                      |
| ---------- | ---------------------- | ------------------------------------------------------------ | ------------------------------------------------------------ |
| 0          | add_identifier         | You can see that each command has an ID. you can nickname any command in this system, including this one, through this command. | At first you can only use `add_identifier` to invoke the command. But if you want to use `happyday` to call the command, then you can do it by executing `add_identifier happyday 0`, where 0 is the ID of the command. After that, every time you execute the `happyday` command, it is equivalent to executing the `add_identifier` command. |
| 1          | delete_identifier      | This command has the opposite function of "add_identifier". This command disables the nickname of a command, including this command. Please note that you should not use this command for "add_identifier" unless you have set another nickname for this command. | Let's continue with the previous command. If we want to disable the nickname `happyday`, we can execute this command `delete_identifier happyday` |
| 2          | switch_version         | With this command you can change the current file version.   | You can use the `version ` command to see the number of the file version you want to switch to. For example, you want to switch to the file version number 4, you can use the `switch_version 4` command to achieve. |
| 3          | make_file              | You can create a new file in the current directory with this command (use the pwd command to see the current path). | For example, you want to create a directory in the `helloworld.cpp` file, then you can execute the `make_file helloworld.cpp` to achieve. |
| 4          | make_dir               | You can create a new folder in the current directory with this command (use the pwd command to see the current path). | For example, you want to create a directory in the `Animal` folder, then you can implement the `make_dir Animal` to achieve. |
| 5          | change_directory       | With this command you can go to a folder in the current directory. | If you want to enter a folder named `Animal`, you can execute `change_director Animal`. |
| 6          | remove_file            | You can delete a file in the current directory with this command. Please note that the deletion must be a file, to delete a folder please use the `remove_dir` command. | If you want to delete a file named `helloworld.cpp`, you can execute `remove_file helloworld.cpp`. |
| 7          | remove_dir             | You can delete a folder in the current directory with this command. Please note that the deletion must be a folder, to delete a file please use the remove_file command. | If you want to delete a folder named `Animal`, you can execute `remove_dir Animal`. |
| 8          | update_name            | You can change the name of a file or folder with this command. | For example, you want to change the name of the file named `Animal` to `Animals`, you can execute `update_name Animal Animals`. |
| 9          | update_content         | With this command you can change the contents of a file.     | For example, you want to change the content of the file named `helloworld.cpp` to `hahaha empty file!`, you can execute `update_content helloworld.cpp hahaha\sempty\sfile!`, where `\s` will be escaped to the space character. The full set of escape characters can be viewed in the command_interpreter of the structure explanation. |
| 10         | get_content            | This command allows you to view the contents of the file.    | For example, you want to view the contents of the helloworld.cpp file, you can execute `get_content helloworld.cpp`. |
| 11         | tree                   | With this command, you can view the tree diagram of the files and folders that make up the current file version. | You can run `tree` to see the tree structure diagram of the current file version. |
| 12         | goto_last_dir          | With this command  you can go back to the previous level of the directory. | If you want to return to the upper level of the directory, you can execute `goto_last_dir`. |
| 13         | list_directory_content | You can view all the files and folders in the current directory and their details with this command. | If you only want to see the files or folders in the current directory, you only need to execute `list_directory_content`. If you also want to see their details, you can follow the addition of a `-a` parameter, that is, the execution of `list_directory_content -a`. |
| 14         | create_version         | This command allows you to create a new a file version. You can either inherit a previous version or create a completely new one. To execute this command you need to name the version you are creating. | For example, if you want to inherit a version of a file version with number 5 and name it `QAQ`. You can execute this command `create_version 5 QAQ`. After that you can use the "switch_version" command to switch. If you want to create a file version without any files or folders, you can directly execute `create_version QAQ`. You can add comments to the version of the file you create. The comments should be placed in the second or third parameter. Please note that special characters such as spaces cannot be present in the comment. If you need to enter such characters, please refer to the command_interpreter in the structure explanation. |
| 15         | version                | With this command you can view all the file versions that exist on the system. This includes the file version number, the nickname of the file version, and the comments you have added to this file version. | Execute `version` to view all the file versions that exist in the system. |
| 16         | get_current_version    | You can view the current file version with this command.     | You can view the current file version with this command.     |
| 17         | init                   | If the use of add_identifier and delete_identifier causes the settings to be wrong, the command cannot be invoked properly, or you just want to restore the initial settings, you can call this command to restore all command nicknames to the initial settings. | If the use of add_identifier and delete_identifier causes the settings to be wrong, the command cannot be invoked properly, or you just want to restore the initial settings, you can call this command to restore all command nicknames to the initial settings. |
| 18         | clear                  | With this command you can clear all the contents displayed in the terminal. | With this command you can clear all the contents displayed in the terminal. |
| 19         | vim                    | With this command, you can invoke vim on your system and thus manipulate files. Please note that to use this command, make sure that vim is installed on your system and that you can use it properly in the terminal. | For example, you want to operate on the `helloworld.cpp` file, you can execute `vim helloworld.cpp` and thus call vim to operate on the file. In vim everything is the same as normal use of vim. When you are done editing, use `:wq` to exit vim to save the edited content to the system. |
| 20         | get_current_path       | With this command you can get the path to the current directory. | With this command you can get the path to the current directory. |
| 21         | find                   | With this command you can search for files or folders in the current file version. Note that the command finds files or folders whose names contain the given string. | For example, if you want to find a file that contains `hello` in its name, you can execute `find hello`. The results of the search are then printed in the terminal. |

## Structure of the system

The system is divided into ten modules. This section contains the role of each module and the calling relationships between the modules. The specific implementation method of each module and the implementation principle of some modules can be viewed in the y principle section.

### Structure Diagram

![structure_diagram](https://github.com/ChantMee/File-Version-Management/blob/master/structure_diagram.png)

### BSTree

This is the storage framework for the entire file system. Provides basic file, folder and path operations for other modules.

### Command Interpreter

This module parses the commands entered by the user. For example, it will escape the user input.

The escapes that currently exist include:

| Before escaping | After escaping |
| --------------- | -------------- |
| \s              | Space          |
| \t              | Tab            |
| \\\             | \              |

### Encryptor

This section provides encryption for the Saver module, thus preventing the user's file information from being stored in plaintext.

### File Manager

This module provides file storage services for node_manager. This module ensures that files with the same content are only stored once in the entire system.

### File System

This module implements the functions that cannot be achieved in BSTree and integrates the functions of other modules. All commands in the terminal correspond to a function in that module. So if another developer wants to rewrite the terminal, it does not need to make any changes to the contents of the module.

### Logger

This module provides logging services for all other modules. The module provides four different levels of logging services：

| Level   | Explanation                                                  |
| ------- | ------------------------------------------------------------ |
| INFO    | Normal logs are used to record what operations the program has performed. At this level, logs are only printed to log files and not to the console. |
| DEBUG   | This level is mainly used when debugging programs. Using this level will output information to the console using `std::cerr`. |
| WARNING | This level is used when the user does something wrong. Please note that if the user did something wrong, the function will only return false. To find out exactly what happened, you can view the logger's `information` variable. |
| FATAL   | This level records information that may cause the program to crash. At this level, information will be forced to be output to the console through `std::cerr` and the log level will be marked. Normally, the program will only issue this warning in extreme cases such as when it detects access to an illegal pointer. |

### Node Manager

This module provides node storage services upwards (BSTree). Downwards it works with File Manager, thus ensuring the proper operation of the system.
The node stores information about the file as well as a hash value which is the hash value of the file stored in File Manager.

### Terminal

This module is used to execute the user commands parsed by the Command Interpreter module.

And the initialization of the whole system, including the setting of command nicknames, etc., will be executed in this module.

### Version Manager

This module serves the File System. all version-related functions are implemented in this module, such as creating file versions, getting a pointer to the root node of the BSTree corresponding to a file version, etc.

### Saver

This module provides storage services for all other modules. Note that the storage mentioned earlier refers to storage in this file system, while the storage provided by this module stores the information in the file system to the physical hard disk.
