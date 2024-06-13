#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <windows.h>
#define MAX_POSSIBLE 16384

static int start = -1;

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

int backup_piece(char str[]) 
{
    FILE* out;
    out = fopen("msg_split_temp.txt", "w");
    fputs(str, out);
    int len = strlen(str);
    return len;
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

int check_piece(int start, FILE *f, int fsize, int LIMIT) 
{
    char tag_open = '<' ; // opening brackets
    char tag_close = '>' ; // closing brackets
    int works;
    char* workable = (char*)malloc(LIMIT*sizeof(char));
    Stack S;
    S.size = 0;
    char stack_top;
    int i, j;
    works = 1;
    i = start;
    while (works && (i<fsize))
    {
        fscanf_s(f, "%c", &workable[i]);
        if (works && (i > 1))
            backup_piece(workable);
        if (workable[i] != '\0')
        {
            for (j = 0; j < 3; j++) // check brackets
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
            }
            i++;
        }
        else
            break;
    }
    if (works && (S.size == 0))
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
        printf("This message length exceeds limit. Splitting.\n");
        //split if possible
    }
}

int main()
{
    int LIMIT = 0;
    do
    {
        scanf_s("%d", &LIMIT);
        msg_split(LIMIT);
        if (LIMIT <= 0)
            printf("Bad input. Required: positive value");
    } while (LIMIT <= 0);
}
