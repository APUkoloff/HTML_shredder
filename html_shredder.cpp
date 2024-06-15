#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <windows.h>
#include <locale.h>
#define MAX_POSSIBLE 16384
#define FUNC_MODE_OPEN 0
#define FUNC_MODE_CLOSE 1

static int start = -1;
static int LIMIT = -1;
static int util_str_index = -1;
static int size1 = 0;
static int size2 = 0;

struct Stack
{
    char* text;
    int size;
};

void Push(Stack& S, char x, int LIMIT)
{
    if ((S.size == LIMIT)||(S.size == MAX_POSSIBLE)) //too big fragment
        return;
    S.text[S.size] = x;
    S.size++;
}

char Pop(Stack& my_stack) // reduce stack & return data function
{
    if (my_stack.size == 0)
    {
        return 255; //error
    }
    my_stack.size--;
    return my_stack.text[my_stack.size];
}

void dump_full_message(FILE* f, int fsize) 
{
    fclose(f);
    f = fopen("source.html", "r");
    if ((fsize <= 0)||(f == NULL))
    {
        printf("Empty file or internal error");
    }
    else 
    {
        char* content = (char*)malloc(fsize * sizeof(char));
        fread(content, sizeof(char), fsize, f);
        if (content != NULL)
        {
            puts(content);
        }
        free(content);
    }
    fclose(f);
}

int html_open_tag_check_part(int offset, int end, char str[], char **picklist) //find meaningful data inside tag brackets
{
    if (picklist == NULL)
        return 0;
    int n = 0;
    for (int j = offset; j < end; j++) 
    {
        if ((str[j] == '\0') || ('/'))
            return 0;
        if ((str[j] == '=')||(str[j]==' ')) // drops links and other from meaningful part
            break;
        picklist[j][n] = str[j];
        n++;
    }
    return 1;
}

int html_close_tag_check_part(int offset, int end, char str[], char** picklist) //check if closing tag really was a CLOSING tag
{
    if (picklist == NULL)
        return 0;
    int n = 0;
    if (str[offset] != ('/'))
            return 0;
    for (int j = offset; j < end; j++)
    {
        picklist[j][n] = str[j];
        n++;
    }
    return 1;
}

int mass_insert_open_tag(int offset, char workable[], int LIMIT, char** picklist)
{
    int j = offset;
    int n = -1;
    int len = 0;
    int base_len = strlen(workable);
    while ((j < LIMIT) || (j < base_len))
    {
        if (workable[j] == '<')
            for (int n = j+1; (n < strlen(workable) || n < LIMIT); n++)
            {
                if ((workable[n] != '/') && (workable[n + 1] == '>'))
                {
                    n++;
                    html_close_tag_check_part(j, n, workable, picklist);
                }
            }
        if (n > j)
            j == n + 1;
        else j++;
    }
    return 1;
}

int mass_insert_close_tag(int offset, char workable[], int LIMIT, char **picklist) 
{
    int j = offset;
    int n = -1;
    int len = 0;
    while (j < LIMIT || j < strlen(workable)) 
    {
        if(workable[j]=='<')
            for(int n = j+1; (n<strlen(workable) || n<LIMIT); n++)
            {
                if ((workable[n] == '/') && (workable[n + 1] == '>'))
                {
                    n++;
                    html_close_tag_check_part(j, n, workable, picklist);
                }
            }    
        if (n > j)
            j == n + 1;
        else j++;
    }  
    return 1;
}

int makearray(char** list, int size)
{
    // Allocate memory for the array of strings
    list = (char**)malloc(size * sizeof(char*));
    if (list == NULL) 
    {
        printf("Memory allocation failed\n");
        return 0;
    }

    for (int i = 0; i < size; i++)
    {
        // Allocate memory for each string
        list[i] = (char*)malloc(80 * sizeof(char));
        if (list[i] == NULL) 
        {
            printf("Memory allocation failed\n");
            return 0;
        }
    }
    return 1;
}

int compare_tag_lists(char** picklist1, char** picklist2, int total1, int total2) 
{
    if ((picklist1 == NULL) || (picklist2 == NULL)) 
    {
        printf("Internal error in compare_tag_lists");
        return 0;
    }
    if (total1 != total2)
        return 0;
    for (int j = 0; j < total1; j++) 
    {
        int len = strlen(picklist1[j]);
        if (len != strlen(picklist2[j]))
            return 0;
        else
            if (strcmp(picklist1[j], picklist2[j])!=0)
                return 0;
    }
    return 1;
}

int backup_piece(char str[], int end) 
{
    if (util_str_index < 0)
        return 0;
    int offset = util_str_index;
    FILE* out;
    out = fopen("msg_split_temp.txt", "w");
    for (int j = offset; j < end; j++) 
    {
        fprintf_s(out, "%c", str[j]);
    }
    fprintf_s(out, "\0");
    fprintf_s(out, "\n");
    return end-offset;
}

int load_piece(int len)
{
    FILE* out;
    out = fopen("msg_split_temp.txt", "r");
    if ((len <= 0)||(out == NULL))
        return 0; // file wasnt created (thus there are no correct fragment)
    else
    {
        char* fragment = (char*)malloc(len*sizeof(char)); // that previously saved(with correct HTML text) element of big message
        fgets(fragment, len, out);
        puts(fragment); // user-requested output
        return 1;
    }
}

int tag_found(int works, int i, FILE *f, int fsize, char workable[], Stack S, char **picklist, int mode) 
{
    char tag_open = '<' ; // opening brackets
    char tag_close = '>' ; // closing brackets
    char stack_top;
    util_str_index = i;
    int tag_open_index = -1;
    int tag_close_index = -1;
    int check1 = 0;
    int check2 = 0;
        
    while (works && (i < fsize))
    {
        fscanf_s(f, "%c", &workable[i], sizeof(char));
        if (workable[i] != '\0')
        {
            if (workable[i] == tag_open) // opening bracket found
            {
                Push(S, workable[i], LIMIT); //add to stack - push this char in array inside
                tag_open_index = i;
                size1++;
                if(FUNC_MODE_OPEN)
                    realloc(picklist, size1 * sizeof(char*));
                else
                    realloc(picklist, size2 * sizeof(char*));
                break;
            }
            if (workable[i] == tag_close) // closing bracket found
            {
                stack_top = Pop(S); // remove that opening bracket(see above) from stack. reduce stack size.
                tag_close_index = i;
                size2++;
                if (stack_top != tag_open)
                    works = 0; // will happen when a)brackets are not paired at all b) closing bracket of pair came before the opening one
                break;
            }
            i++;
        }
        else
            break;
    }
    if (works) 
    {
        if(FUNC_MODE_OPEN)
            mass_insert_open_tag(util_str_index, workable, LIMIT, picklist);
        if (FUNC_MODE_CLOSE)
            mass_insert_close_tag(util_str_index, workable, LIMIT, picklist);
        return 1;
    }
            
    else 
    { 
        util_str_index = -1;
        return -1; 
    }
}

int check_piece(int start, FILE *f, int fsize, int LIMIT) 
{
    int works;
    char* workable = (char*)malloc(LIMIT*sizeof(char));
    Stack S1;
    Stack S2;
    S1.size = 0;
    S2.size = 0;
    works = 1;
    int i = start;
    if (fsize < LIMIT) 
    {
        dump_full_message(f, fsize);
        return 42;
    }
    char** picklist1;
    char** picklist2;
    // Allocate memory for the array of strings
    picklist1 = (char**)malloc(1 * sizeof(char*));
    if (picklist1 == NULL)
    {
        printf("Memory allocation failed\n");
        return 220;
    }
    picklist1[0] = (char*)malloc(80 * sizeof(char));
    if (picklist1[0] == NULL)
    {
        printf("Memory allocation failed\n");
        return 220;
    }
    picklist2 = (char**)malloc(1 * sizeof(char*));
    if (picklist2 == NULL)
    {
        printf("Memory allocation failed\n");
        return 220;
    }
    picklist2[0] = (char*)malloc(80 * sizeof(char));
    if (picklist2[0] == NULL)
    {
        printf("Memory allocation failed\n");
        return 220;
    }
    works = tag_found(works, i, f, fsize, workable, S1, picklist1, FUNC_MODE_OPEN);
    works = tag_found(works, i, f, fsize, workable, S1, picklist2, FUNC_MODE_CLOSE);
    works = compare_tag_lists(picklist1, picklist2, size1, size2);
    if (works && (S1.size == 0))
    {
        if(workable!=NULL)
            puts(workable);
        start = i;
        free(picklist1);
        free(picklist2);
        return 0;
    }
    else
    {
        puts("Not implemented yet\n");
        //split if possible
        free(picklist1);
        free(picklist2);
        return 1;
    }
    
}

int msg_split(int LIMIT) 
{
    FILE *f;
    f = fopen("source.html", "r");
    if (f == NULL)
        return 404;
    fseek(f, 0, SEEK_END);  //find eof
    int fsize = ftell(f) / sizeof(char);
    int res = check_piece(start, f, fsize, LIMIT);
    if (res == 1) 
    {
        printf("This message length exceeds limit. Splitted.\n");
        //split if possible
    }
    if (res == 42) 
    {
        printf("The message above fullfills size limit and was printed as is\n");
    }
    fclose(f);
    return 0;
}

int main()
{
    setlocale(LC_ALL, "en_US.UTF-8");
    do
    {
        scanf_s("%d", &LIMIT);
        if (LIMIT <= 0)
            printf("Bad input. Required: positive value");
        
    } while (LIMIT <= 0);
    if (msg_split(LIMIT) == 404)
        printf("Source file not found\n");
    else
        printf("Completed\n");
    system("Pause");
}
