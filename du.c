#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "kernel/fs.h"

/* Set NULL pointer */
#define NULL ((void*)0)
/* Create struct store the information of file */
struct file_info{
    char fname[512];                // Name of the file
    int dmem;                   // Disk usage of the file
    int amem;                   // Actual size of the file
    int block_num;              // Block size of the file
};
/* Create linked list */
typedef struct file_store{
    struct file_info f;
    struct file_store *next;
}file_store;
/* Linked list initialization */
file_store *init(){
    file_store *head = (file_store*)malloc(sizeof(file_store));
    head->next = NULL;
    return head;
}
/* Add item to linked list from head */
void push(file_store *head,struct file_info x){
    file_store *newnode = (file_store*)malloc(sizeof(file_store));
    newnode->next = head->next;
    newnode->f.amem = x.amem;
    newnode->f.dmem = x.dmem;
    newnode->f.block_num = x.block_num;
    strcpy(newnode->f.fname, x.fname);
    head->next = newnode;
}
/* Global Variable */
int sum = 0;                    // Sum of the size on the disk
int flag;                       // Flags of du command
int k_flag = 0;                 // "-k"
int t_flag = 0;                 // "-t"
int t_num = 0;                  // "-t [threshold]"
int f_flag = 0;                 // "[File]"
int d_flag = 0;                 // "[Directory]"
int total_dmem = 0;             // Total size of disk mem
int total_block_num = 0;        // Total size of block number 
/* Function Header */
uint diskmem(uint);             // Find the size on the disk
int block_size(int);            // Find the block size
char *fmtname(char*);           // Name of the file
char *strcat(char*, char*);     // Connect two char*
char lastDigit(char*);         // Find the last digit of a string
void du(char*, struct file_info, struct file_store *);      
void output(struct file_store *, char *);          
/*-------------------------------------------------------------------------------------------------------------------------------*/
/* Find the size on the disk */
uint diskmem(uint amem){
    // Whether actual file size could be divided by 512
    uint num = amem % 512;
    uint dmem;
    if (num != 0)   // Could not be divisible, fill in the remaining byte
        dmem = amem + (512 - num);
    else    // Could be divisible
        dmem = amem;
    return dmem;
}
/* Find number of blocks */
int block_size(int amem){
    int num_b = (amem + 512) / 512;
    return num_b;
}
/* Find name of the file */
char *fmtname(char *path) {
    static char buf[DIRSIZ + 1];
    char *p;

    // Find first character after last slash.
    for (p = path + strlen(path); p >= path && *p != '/'; p--);
    p++;

    // Return blank-padded name.
    if (strlen(p) >= DIRSIZ)
        return p;
    memmove(buf, p, strlen(p));
    memset(buf + strlen(p), ' ', DIRSIZ - strlen(p));
    return buf;
}
/* Connect two char* */
char *strcat(char *des, char *src){
    if ((des == NULL) || (src == NULL))   return NULL;  
    char *temp = des;       // Remember starting address
    while (*des != '\0') {  // Make the pointer points to the end of des
        des++;
    }
    while (*src != '\0') {  // Adding src to des
        *des++ = *src++;
    }
    *des = '\0';            // Ending with \0
    return temp;
} 
/* Find the last digit of string */
char lastDigit(char *src){
    if (src == NULL) return '0';
    int len = strlen(src);
    char result = src[len-1];
    return result;
}
/* DU function */
void du(char *path, struct file_info file_item, struct file_store *all_file) {
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;
    
    /* Checking path validation */
    if ((fd = open(path, 0)) < 0) {
        printf(2, "%s\n", "check usage.");   // Error message--wrong addr/dir
        exit();
    }
    if (fstat(fd, &st) < 0) {
        printf(2, "%s\n", "check usage.");        // Error message--invalid status of file
        close(fd);
        exit();
    }

    switch (st.type) {
        case T_FILE:
            /* Modify features of file's information and store to the linked list */
            file_item.dmem = diskmem(st.size);
            file_item.amem = st.size;
            strcpy(file_item.fname, fmtname(buf));
            file_item.block_num = block_size(st.size);
            push(all_file, file_item);      // Add to linked list 
            break;

        case T_DIR:
            if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
                printf(2, "%s\n", "check usage."); /* Error Message */
                exit();
            }
            strcpy(buf, path);
            p = buf + strlen(buf);
            *p++ = '/';
            while (read(fd, &de, sizeof(de)) == sizeof(de)) {
                if (de.inum == 0)
                    continue;
                memmove(p, de.name, DIRSIZ);
                p[DIRSIZ] = 0;
                if (stat(buf, &st) < 0) {
                    printf(2, "%s\n", "check usage.");
                    continue;
                }
                if (st.type == 2) {
                    /* Modify features of file's information and store to the linked list */
                    file_item.dmem = diskmem(st.size);
                    file_item.amem = st.size;
                    strcpy(file_item.fname, fmtname(buf));
                    file_item.block_num = block_size(st.size);
                    push(all_file, file_item);  // Add to linked list
                }
            }
            break;
        }
        close(fd);
}
/* Output results according to the input flags */
void output(file_store *head, char *path){
    file_store *temp = head->next;
    if (k_flag == 1) {
        if (t_flag == 0) {          // -k -> block number
            while (temp != NULL) {
                if (strcmp(path, ".") == 0) {
                    printf(1, "%d %s%s\n", temp->f.block_num, "./", temp->f.fname);
                }
                else {
                    char m = lastDigit(path);
                    char *n = &m;
                    if (strcmp(n, "/") != 0) {
                        printf(1, "%d %s%s%s\n", temp->f.block_num, path, "/", temp->f.fname);
                    }
                    else {
                        printf(1, "%d %s%s\n", temp->f.block_num, path, temp->f.fname);
                    }
                }
                /* Add to the total number of block num */
                total_block_num = total_block_num + temp->f.block_num;
                temp = temp->next;
            }
        }
        if (t_flag == 1) {          // -k -t -> block number & limits of actual mem
            while (temp != NULL) {
                if (temp->f.amem >= t_num) {
                    if (strcmp(path, ".") == 0) {
                        printf(1, "%d %s%s\n", temp->f.block_num, "./", temp->f.fname);
                    }
                    else {
                        char m = lastDigit(path);
                        char *n = &m;
                        if (strcmp(n, "/") != 0) {
                            printf(1, "%d %s%s%s\n", temp->f.block_num, path, "/", temp->f.fname);
                        }
                        else {
                            printf(1, "%d %s%s\n", temp->f.block_num, path, temp->f.fname);
                        }
                    }
                    /* Add to the total number of block num */
                    total_block_num = total_block_num + temp->f.block_num;
                }
                temp = temp->next;
            }
        }
        printf(1, "%d %s\n", total_block_num, ".");
    }
    if (k_flag == 0) {
        if (t_flag == 0) {          // none -> disk mem
            while (temp != NULL) {
                if (strcmp(path, ".") == 0) {
                    printf(1, "%d %s%s\n", temp->f.dmem, "./", temp->f.fname);
                }
                else {
                    char m = lastDigit(path);
                    char *n = &m;
                    if (strcmp(n, "/") != 0) {
                        printf(1, "%d %s%s%s\n", temp->f.dmem, path, "/", temp->f.fname);
                    }
                    else {
                        printf(1, "%d %s%s\n", temp->f.dmem, path, temp->f.fname);
                    }
                }
                /* Add to the total number of disk mem */
                total_dmem = total_dmem + temp->f.dmem;
                temp = temp->next;
            }
        }
        if (t_flag == 1) {          // -t -> disk mem & limits of actual mem
            while (temp != NULL) {
                if (temp->f.amem >= t_num) {
                    if (strcmp(path, ".") == 0) {
                        printf(1, "%d %s%s\n", temp->f.dmem, "./", temp->f.fname);
                    }
                    else {
                        char m = lastDigit(path);
                        char *n = &m;
                        if (strcmp(n, "/") != 0) {
                            printf(1, "%d %s%s%s\n", temp->f.dmem, path, "/", temp->f.fname);
                        }
                        else {
                            printf(1, "%d %s%s\n", temp->f.dmem, path, temp->f.fname);
                        }
                        
                    }
                    /* Add to the total number of disk mem */
                    total_dmem = total_dmem + temp->f.dmem;
                }
                temp = temp->next;
            }
        }
        printf(1, "%d %s\n", total_dmem, ".");
    }
    return;
}

/*-------------------------------------------------------------------------------------------------------------------------------*/
int main(int argc, char *argv[]) {
    /* Declare struct to store all the information of one specific file and initialize */
    struct file_info file_item;
    file_item.amem = 0;
    file_item.block_num = 0;
    file_item.dmem = 0;
    strcpy(file_item.fname, "");
    /* Declare linked list to store all files and initialize */
    struct file_store *all_file = init();
    struct file_store *head = all_file;
    int m;
    /* Initialize path */
    char *path;
    path = ".";
    /* Input: du */
    if (argc < 2) {
        du(".", file_item, head);
        output(head, path);
        exit();
    }
    /* Input: du + flags */
    // Set flags according to command
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-k") == 0) { // -k
            if ((i+1)<=argc) {
                // -k connected with a number, error
                m = atoi(argv[i+1]);
                if (m > 0) {
                    printf(2, "%s\n", "check usage.");
                    exit();
                }
            }
            if (k_flag) {
                printf(2, "%s\n", "check usage.");  // Error message when input duplicate -k
                exit();
            }
            k_flag = 1;
        }
        else if (strcmp(argv[i], "-t") == 0){    // -t
            if ((i+1) <= argc) {
                // -t connected without an integer(negative or float)
                m = atoi(argv[i+1]);
                if ((m < 0)||(m == 0)) {
                    printf(2, "%s\n", "check usage.");      // Error message
                    exit();
                }
                if (t_flag) {
                    printf(2, "%s\n", "check usage.");  // Error message when input duplicate -t
                    exit();
                }
                t_flag = 1;
                t_num = m; 
                i++;
            }
            else {
                printf(2, "%s\n", "check usage.\n");        // Error message
                exit();
            }
        }
        else {
            /* All other situation including either -k followed by a number or a none-exist path will be 
                checked in du() and printed corresponding error message */
            path = argv[i];
        }
    }
    du(path, file_item, head);
    output(head, path);
    exit();
}
