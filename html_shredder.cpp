#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <windows.h>
#define MAX_POSSIBLE 16384

static int start = -1;
static int LIMIT = -1;
static int util_str_index = -1;

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

int opening_tag_found(int works, int i, FILE *f, int fsize, char workable[], Stack S) 
{
    char tag_open = '<' ; // opening brackets
    char tag_close = '>' ; // closing brackets
    char stack_top;
    util_str_index = i;
    while (works && (i < fsize))
    {
        fscanf_s(f, "%c", &workable[i]);
        if (workable[i] != '\0')
        {
            if (workable[i] == tag_open) // opening bracket found
            {
                Push(S, workable[i], LIMIT); //add to stack - push this char in array inside
                break;
            }
            if (workable[i] == tag_close) // closing bracket found
            {
                stack_top = Pop(S); // remove that opening bracket(see above) from stack. reduce stack size.
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
        return 1;
    else 
    { 
        util_str_index = -1;
        return 0; 
    }
}

int closing_tag_found(int works, int i, FILE* f, int fsize, char workable[], Stack S)
{
    if (works == 0)
        return 0;
    char tag_open = '<'; // opening brackets
    char tag_close = '>'; // closing brackets
    char stack_top;
    while (works && (i < fsize))
    {
        fscanf_s(f, "%c", &workable[i]);
        if (works && (i > 1))
            backup_piece(workable, i);
        if (workable[i] != '\0')
        {
            if (workable[i] == tag_open) // opening bracket found
            {
                Push(S, workable[i], LIMIT); //add to stack - push this char in array inside
                break;
            }
            if (workable[i] == tag_close) // closing bracket found
            {
                stack_top = Pop(S); // remove that opening bracket(see above) from stack. reduce stack size.
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
        return 1;
    else return 0;
}

int check_piece(int start, FILE *f, int fsize, int LIMIT) 
{
    
    int works;
    char* workable = (char*)malloc(LIMIT*sizeof(char));
    Stack S1;
    Stack S2;
    S1.size = 0;
    S2.size = 0;
    int i, j;
    works = 1;
    i = start;
    if (fsize < LIMIT) 
    {
        dump_full_message(f, fsize);
        return 42;
    }
    works = opening_tag_found(works, i, f, fsize, workable, S1);
    works = closing_tag_found(works, i, f, fsize, workable, S1);
    if (works && (S1.size == 0))
    {
        if(workable!=NULL)
            puts(workable);
        start = i;
        return 0;
    }
    else
    {
        //split if possible
        return 1;
    }
}

int msg_split(int LIMIT) 
{
    FILE *f;
    f = fopen("source.html", "r");
    if (f == NULL)
        return 404;
    fseek(f, 0, SEEK_END);  // устанавливаем указатель на конец файла
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
    return 0;
}

int main()
{
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
}
