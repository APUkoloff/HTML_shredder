#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * This function calculates the middle of the text, splits it into 2 parts and returns them
 * Keeps the word boundaries and takes care of the HTML tags too! There is no broken HTML tag after the split.
 */
void splitHtmlTextIntoTwoEqualColumns(char* htmlText, char** column1, char** column2)
{
    // removes unnecessary characters and HTML tags
    char* temp = str_replace(htmlText, "\xc2\xa0", " ");
    char* decodedText = html_entity_decode(temp);
    free(temp);
    char* pureText = getPureText(decodedText);
    free(decodedText);

    // calculates the length of the text
    int fullLength = strlen(pureText);
    int halfLength = (fullLength + 1) / 2;

    char** words = str_split(pureText, ' ');

    // finds the word which is in the middle of the text
    int middleWordPosition = getPositionOfMiddleWord(words, halfLength);

    // iterates through the HTML and split the text into 2 parts when it reaches the middle word.
    char** columns = splitHtmlStringInto2Strings(htmlText, middleWordPosition);

    char** closedColumns = closeUnclosedHtmlTags(columns, halfLength * 2);
    *column1 = closedColumns[0];
    *column2 = closedColumns[1];

    free(pureText);
    free_str_array(words);
    free_str_array(columns);
    free(closedColumns);
}

char* getPureText(char* htmlText)
{
    char* pureText = strip_tags(htmlText);
    char* temp = remove_control_chars(pureText);
    free(pureText);
    pureText = str_replace(temp, "\r\n", "");
    pureText = str_replace(pureText, "\r", "");
    pureText = str_replace(pureText, "\n", "");
    free(temp);
    return pureText;
}

/**
 * finds the word which is in the middle of the text
 */
int getPositionOfMiddleWord(char** words, int halfLength)
{
    int wordPosition = 0;
    int stringLength = 0;
    int p;
    for (p = 0; words[p] != NULL; p++)
    {
        stringLength += strlen(words[p]) + 1;
        if (stringLength > halfLength)
        {
            wordPosition = p;
            break;
        }
    }
    return wordPosition;
}

char** splitHtmlStringInto2Strings(char* htmlText, int wordPosition)
{
    char** columns = (char**)malloc(sizeof(char*) * 3);
    columns[0] = (char*)calloc(1, sizeof(char));
    columns[1] = (char*)calloc(1, sizeof(char));
    columns[2] = NULL;
    int columnId = 0;
    int wordCounter = 0;
    int inHtmlTag = 0;
    int s;
    for (s = 0; htmlText[s] != '\0'; s++)
    {
        if (!inHtmlTag && htmlText[s] == '<')
        {
            inHtmlTag = 1;
        }
        if (inHtmlTag)
        {
            columns[columnId] = str_append(columns[columnId], htmlText[s]);
            if (htmlText[s] == '>')
            {
                inHtmlTag = 0;
            }
        }
        else
        {
            if (htmlText[s] == ' ')
            {
                wordCounter++;
            }
            if (wordCounter > wordPosition && columnId < 1)
            {
                columnId++;
                wordCounter = 0;
            }
            columns[columnId] = str_append(columns[columnId], htmlText[s]);
        }
    }
    columns[0] = trim(columns[0]);
    columns[1] = trim(columns[1]);
    return columns;
}

char** closeUnclosedHtmlTags(char** columns, int maxLength)
{
    char* column1 = columns[0];
    char** unclosedTags = getUnclosedHtmlTags(column1, maxLength);
    int i;
    for (i = 0; unclosedTags[i] != NULL; i++)
    {
        char* closingTag = (char*)malloc(strlen(unclosedTags[i]) + 4);
        sprintf(closingTag, "</%s>", unclosedTags[i]);
        column1 = str_append(column1, *closingTag); //*
        free(closingTag);
    }
    char* column2 = (char*)calloc(1, sizeof(char));
    for (i = 0; unclosedTags[i] != NULL; i++)
    {
        char* openingTag = (char*)malloc(strlen(unclosedTags[i]) + 3);
        sprintf(openingTag, "<%s>", unclosedTags[i]);
        column2 = str_append(column2, *openingTag); //*
        free(openingTag);
    }
    column2 = str_append(column2, *columns[1]); //*
    char** closedColumns = (char**)malloc(sizeof(char*) * 3);
    closedColumns[0] = column1;
    closedColumns[1] = column2;
    closedColumns[2] = NULL;
    free_str_array(unclosedTags);
    return closedColumns;
}

char** getUnclosedHtmlTags(char* html, int maxLength)
{
    int htmlLength = strlen(html);
    char** unclosed = (char**)malloc(sizeof(char*) * (maxLength + 1));
    int counter = 0;
    int i = 0;
    int j = 0;
    while (i < htmlLength && counter < maxLength)
    {
        if (html[i] == '<') {
            i++;
            if (i < htmlLength && html[i] != '/')
            {
                char* currentTag = (char*)calloc(1, sizeof(char));
                while (i < htmlLength && html[i] != '>' && html[i] != '/')
                {
                    currentTag = str_append(currentTag, html[i]);
                    i++;
                }
                if (html[i] == '/')
                {
                    while (i < htmlLength && html[i] != '>')
                    {
                        i++;
                    }
                }
                else
                {
                    char** tagParts = str_split(currentTag, ' ');
                    unclosed[j++] = strdup(tagParts[0]);
                    free_str_array(tagParts);
                    free(currentTag);
                }
            }
            else if (html[i] == '/') {
                i++;
                while (i < htmlLength && html[i] != '>')
                {
                    i++;
                }
                j--;
            }
        }
        else
        {
            counter++;
        }
        i++;
    }
    unclosed[j] = NULL;
    return unclosed;
}

// Helper functions (not included in the original PHP code)
char* str_replace(char* str, char* oldStr, char* newStr)
{
    char* result = strdup(str);
    char* ins = result;
    char* tmp = NULL;
    int len_rem = strlen(str);
    int len_old = strlen(oldStr);
    int len_new = strlen(newStr);
    int count = 0;

    while (len_rem >= len_old)
    {
        tmp = strstr(ins, oldStr);
        if (tmp == NULL)
        {
            break;
        }
        count++;
        len_rem -= len_old;
        tmp += len_old;
        memmove(ins + len_new, tmp, len_rem + 1);
        memcpy(ins, newStr, len_new);
        ins += len_new;
    }

    return result;
}

char* html_entity_decode(char* str)
{
    // Implementation of html_entity_decode function
    // ...
    return str;
}

char* strip_tags(char* str)
{
    // Implementation of strip_tags function
    // ...
    return str;
}

char* remove_control_chars(char* str)
{
    char* result = strdup(str);
    char* d = result;
    char* s = str;
    while (*s) {
        if (isprint(*s))
        {
            *d++ = *s;
        }
        s++;
    }
    *d = '\0';
    return result;
}

char** str_split(char* str, char delim)
{
    char** result = NULL;
    char* s = str;
    int count = 0;
    char* tmp = NULL;

    while (*s)
    {
        if (*s == delim)
        {
            count++;
            *s = '\0';
        }
        s++;
    }

    result = (char**)malloc(sizeof(char*) * (count + 2));
    s = str;
    count = 0;
    while (*s)
    {
        tmp = s;
        s = strchr(s, '\0') + 1;
        result[count++] = strdup(tmp);
    }
    result[count] = NULL;

    return result;
}

char* str_append(char* str, char c)
{
    int len = strlen(str);
    char* result = (char*)realloc(str, len + 2);
    result[len] = c;
    result[len + 1] = '\0';
    return result;
}

char* trim(char* str)
{
    char* end;
    while (isspace(*str))
    {
        str++;
    }
    end = str + strlen(str) - 1;
    while (end >= str && isspace(*end))
    {
        end--;
    }
    *(end + 1) = '\0';
    return strdup(str);
}

void free_str_array(char** arr)
{
    int i;
    for (i = 0; arr[i] != NULL; i++)
    {
        free(arr[i]);
    }
    free(arr);
}

int main()
{
    char text[] = "<!-- This is an HTML comment --><p>This is a <b>paragraph</b> with some <i>formatted</i> text.</p>";
    char* htmlText = text;
    char* column1, * column2;
    splitHtmlTextIntoTwoEqualColumns(htmlText, &column1, &column2);
    printf("Column 1: %s\n", column1);
    printf("Column 2: %s\n", column2);
    free(column1);
    free(column2);
    return 0;
}
