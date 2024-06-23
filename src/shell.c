#include "shell.h"
#include "kernel.h"
#include "std_lib.h"
#include "filesystem.h"

void shell() {
    char buf[64];
    char cmd[64];
    char arg[2][64];
    byte cwd = FS_NODE_P_ROOT;

    while (true) {
        printString("MengOS:");
        printCWD(cwd);
        printString("$ ");
        readString(buf);
        parseCommand(buf, cmd, arg);

        if (strcmp(cmd, "cd")) {
            cd(&cwd, arg[0]);
        } else if (strcmp(cmd, "ls")) {
            ls(cwd, arg[0]);
        } else if (strcmp(cmd, "mv")) {
            mv(cwd, arg[0], arg[1]);
        } else if (strcmp(cmd, "cp")) {
            cp(cwd, arg[0], arg[1]);
        } else if (strcmp(cmd, "cat")) {
            cat(cwd, arg[0]);
        } else if (strcmp(cmd, "mkdir")) {
            mkdir(cwd, arg[0]);
        } else if (strcmp(cmd, "clear")) {
            clearScreen();
        } else {
            printString("Invalid command\n");
        }
    }
}

void printCWD(byte cwd) {
    struct node_fs node_fs_buf;
    byte path[64][16];
    int path_length = 0;
    int i;

    if (cwd == FS_NODE_P_ROOT) {
        printString("/");
        return;
    }

    readSector(&node_fs_buf, FS_NODE_SECTOR_NUMBER);

    while (cwd != FS_NODE_P_ROOT) {
        strcpy(path[path_length], node_fs_buf.nodes[cwd].node_name);
        cwd = node_fs_buf.nodes[cwd].parent_index;
        path_length++;
    }

    for (i = path_length - 1; i >= 0; i--) {
        if (i < path_length - 1) {
            printString("/");
        }
        printString(path[i]);
    }
}

void parseCommand(char* buf, char* cmd, char arg[2][64]) {
    int i = 0;
    int j = 0;
    int arg_index = 0;
    int buf_length = strlen(buf);

    cmd[0] = '\0';
    arg[0][0] = '\0';
    arg[1][0] = '\0';

    while (i < buf_length && buf[i] != ' ') {
        cmd[i] = buf[i];
        i++;
    }
    cmd[i] = '\0';

    while (i < buf_length && buf[i] == ' ') {
        i++;
    }

    while (i < buf_length && arg_index < 2) {
        j = 0;
        while (i < buf_length && buf[i] != ' ') {
            arg[arg_index][j] = buf[i];
            i++;
            j++;
        }
        arg[arg_index][j] = '\0';
        arg_index++;

        while (i < buf_length && buf[i] == ' ') {
            i++;
        }
    }
}

void cd(byte* cwd, char* dirname) {
    struct node_fs node_fs_buf;
    int i;

    readSector(&node_fs_buf, FS_NODE_SECTOR_NUMBER);

    if (strcmp(dirname, "/")) {
        *cwd = FS_NODE_P_ROOT;
        return;
    }

    if (strcmp(dirname, "..")) {
        if (*cwd != FS_NODE_P_ROOT) {
            *cwd = node_fs_buf.nodes[*cwd].parent_index;
        }
        return;
    }

    for (i = 0; i < FS_MAX_NODE; i++) {
        if (node_fs_buf.nodes[i].parent_index == *cwd && strcmp(node_fs_buf.nodes[i].node_name, dirname)) {
            if (node_fs_buf.nodes[i].data_index == FS_NODE_D_DIR) { // Ensure it is a directory
                *cwd = i;
                return;
            } else {
                printString("Error: Not a directory\n");
                return;
            }
        }
    }

    printString("Error: Directory not found\n");
}

void ls(byte cwd, char* dirname) {
    struct node_fs node_fs_buf;
    int i;
    readSector(&node_fs_buf, FS_NODE_SECTOR_NUMBER);

    for (i = 0; i < FS_MAX_NODE; i++) {
        if (node_fs_buf.nodes[i].parent_index == cwd) {
            if (node_fs_buf.nodes[i].node_name[0] != '\0') {
                int j = 0;
                while (node_fs_buf.nodes[i].node_name[j] != '\0') {
                    if (node_fs_buf.nodes[i].node_name[j] < 32 || node_fs_buf.nodes[i].node_name[j] > 126) {
                        break;
                    }
                    j++;
                }
                if (node_fs_buf.nodes[i].node_name[j] == '\0') {
                    printString(node_fs_buf.nodes[i].node_name);

                    if (node_fs_buf.nodes[i].data_index == FS_NODE_D_DIR) {
                        printString(" ");
                    } else {
                        printString(" ");
                    }
                }
            }
        }
    }
    printString("\n");
}

void mv(byte cwd, char* src, char* dst) {
    // Function implementation goes here
}

void cp(byte cwd, char* src, char* dst) {
    // Function implementation goes here
}

void cat(byte cwd, char* filename) {
    struct node_fs node_fs_buf;
    struct data_fs data_fs_buf;
    struct node_item* file_node = NULL;
    int i;
    char buf[SECTOR_SIZE];

    readSector(&node_fs_buf, FS_NODE_SECTOR_NUMBER);
    readSector(&data_fs_buf, FS_DATA_SECTOR_NUMBER);

    for (i = 0; i < FS_MAX_NODE; i++) {
        if (node_fs_buf.nodes[i].parent_index == cwd &&
            strcmp(node_fs_buf.nodes[i].node_name, filename)) {

            file_node = &node_fs_buf.nodes[i];
            break;
        }
    }

    if (file_node == NULL) {
        printString("File not found\n");
        return;
    }

    if (file_node->data_index == FS_NODE_D_DIR) {
        printString("Error: Not a file\n");
        return;
    }

    for (i = 0; i < FS_MAX_SECTOR; i++) {
        if (data_fs_buf.datas[file_node->data_index].sectors[i] == 0x00) {
            break;
        }
        readSector(buf, data_fs_buf.datas[file_node->data_index].sectors[i]);
        printString(buf);
        printString("\n");
    }
}

void mkdir(byte cwd, char* dirname) {
    struct node_fs node_fs_buf;
    int i;
    char temp_buf[64];

    readSector(&node_fs_buf, FS_NODE_SECTOR_NUMBER);

    for (i = 0; i < FS_MAX_NODE; i++) {
        if (node_fs_buf.nodes[i].parent_index == cwd &&
            strcmp(node_fs_buf.nodes[i].node_name, dirname)) {
            printString("Error: Directory already exists\n");
            return;
        }
    }

    for (i = 0; i < FS_MAX_NODE; i++) {
        if (node_fs_buf.nodes[i].node_name[0] == '\0') {
            strcpy(node_fs_buf.nodes[i].node_name, dirname);
            node_fs_buf.nodes[i].parent_index = cwd;
            node_fs_buf.nodes[i].data_index = FS_NODE_D_DIR;

            writeSector(&node_fs_buf, FS_NODE_SECTOR_NUMBER);
            return;
        }
    }

    printString("Error: No free nodes available\n");
}
