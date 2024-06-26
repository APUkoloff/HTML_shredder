// msg_split.cpp : This is a lib version of msg_split
//
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <stdbool.h>
#define MAX_LEN_HARD_LIMIT 4098
//#define DEBUGGING_SPLITTER 1
#define DROP_STDOUT 1 // stdout output 0 for html output (debug option)
#define SPLIT_TEST_LAUNCHES_NUMBER 0 // ammount of output files after which program work is stopped
#define CLEANUP_MSG_START_DIR_WIDE 0 // to be implemented
#define BAD_LAUNCH_MSG_CLEANUP 0 // to be implemented
// used to prevent mass spam of work folder with output files

// PROBLEMS:
// 1. Will apply wrong formatting on block sometimes. Plain text can become italics for example (all fragment, not what was suuposed to be italicised). That's because of autocloser func. 
// 2. What if html files already had been created after last run and weren't cleaned? (They will be overwritten and it is NOT a problem and NOT MEANT TO BE FIXED).
// 3. Didn't test what happens if message size is too small(no messager will forcibly cut message to 3 characters BUT somebody should do something about it). 
// UPD: 3 drops error after about 30 files. It autocloses 1-char fragments. heh. Need to add some failsafe in tag_closer as well.
// Verdict: releasable I guess. Some test scripts and conversion to h-file are to be done later. Upd: lib done
//static int launches = 0; // used for debugging
int file_size(char fn[])
{
	FILE* f;
	f = fopen("source.html", "r");
	if (f == NULL)
		return -1;
	fseek(f, 0, SEEK_END);  //find eof
	return ftell(f) / sizeof(char);
}

int tag_closer(int tags_len_frag, char* stack, char* output, int output_size, int max_len, int fragment_num) // does exactly what is labeled on the tin
{
	for (int i = tags_len_frag - 1; i >= 0; i--)
	{
		char aux[500] = "";
		strcpy(aux, &stack[i]);
		if (aux[1] != '/')
		{
			int pos = strlen(aux);
			for (int i = pos; i > 0; i--)
			{
				aux[i] = aux[i - 1];
			}
			aux[1] = '/'; // aux[0] should be '<' obviously
		}
		strcpy(output + output_size + 1, aux);
		output_size = output_size + strlen(aux);
		//if (output_size > max_len) // removed this for "release" version
			//printf("bypassed length limit in block %d (BUG#1 is involved)\n", fragment_num); //debug warning
	}
	return output_size;
}

int drop_buffer(int fragment_num, int tags_len_frag, char* stack, char* output, int output_size, int max_len) // outputs messages in html format
{
	output_size = tag_closer(tags_len_frag, stack, output, output_size, max_len, fragment_num);
	if (output == NULL)
	{
		printf("Failed output: %d", fragment_num);
		return 0;
	}
	if (output_size > max_len) 
	{
		printf("This fragment was too large: %d\n", fragment_num);
		return 0;
	}
	char fo_name[500] = "";
	snprintf(fo_name, 500, " %s%i.html", "message_part-", fragment_num); // used to print format string to stream(name)
	int len = strlen(output);
	if (DROP_STDOUT)
	{
		printf("fragment #%d: %d chars(raw)\n", fragment_num, output_size);
		puts(output);
	}

	else
	{
		FILE* html_divided = fopen(fo_name, "w");
		fputs(output, html_divided);
		fclose(html_divided);
	}
	//launches++; //debugging applicable
	return len;
}

int msg_split(char fname[], int max_len, int DEBUG)
{
	int fsize = file_size(fname);
	if (fsize == -1)
	{
		printf("Unable to open %s\n", fname);
		return 0;
	}
	if (fsize == 0)
	{
		printf("Empty or non-readable file: %s\n", fname);
		return 0;
	}
	FILE* html_f = fopen(fname, "r");
	if (html_f == NULL)
	{
		printf("Error opening %s", fname);
		return 0;
	}
	if (max_len > MAX_LEN_HARD_LIMIT)
	{
		printf("Fragment length exceeds maximum possible length. Splitting cancelled.\n");
		return 0;
	}
	if (max_len <= 16)
	{
		printf("Fragment_length is too small. Block tag closer facility cannot be used properly\n");
		return 0;
	}
	char tags_opening[8][9] = { "<b>", "<p>", "<div>","<strong>", "<i>", "<ul>", "<ol>",  "<span>" }; //opening block tags list
	char tags_closing[8][10] = { "</b>", "</p>", "</div>", "</strong>", "</i>", "</ul>", "</ol>", "</span>" }; // closing block tags list
	char curr;
	char output[4098] = "";
	int output_arr_index = 0;
	int output_size = 0;
	char stack[100][100]; // Stack for block tags
	int stack_size = 0;
	int index_s = 0;
	char tag_title[500] = "";
	//int processed = 0; // has been used to try to get something done about bugs listed above
	int tag_index = 0;
	bool tag_opened = false; // yes or no
	int tags_len_frag = 0;
	int fragment_num = 0;
	fseek(html_f, 0, SEEK_SET);
	for (int i = 0; i < 100; i++) // fill all stack
	{
		memset(stack[i], '\0', 100);
	}
	while (1) // if all symbols in file are processed or block length is max results will be dumped in html file with block index
	{
		curr = fgetc(html_f);
		if (curr == -1) // drop full message
		{
			if (drop_buffer(fragment_num, tags_len_frag, *stack, output, output_size, max_len) == 0)
				return 0;
			break;
		}
		else if (curr == '<') // start recording tag name
		{
			tag_opened = true;
			tag_title[tag_index] = curr;
			tag_index++;
		}
		else if (curr == '>') // tag end, check if it was a block tag
		{
			tag_opened = false;
			tag_title[tag_index] = curr;
			for (int i = 0; i < 8; i++)
			{
				if (strcmp(tags_opening[i], tag_title) == 0)
				{
					strcpy(stack[index_s], tag_title);
					stack_size = stack_size + strlen(tag_title);
					tags_len_frag++;
					index_s++;
				}
			}
			for (int i = 0; i < 8; i++)
			{
				if (strcmp(tags_closing[i], tag_title) == 0)
				{
					strcpy(stack[index_s], tag_title);
					stack_size = stack_size + strlen(tag_title);
					tags_len_frag++;
					index_s++;
					for (int j = 0; j < 8; j++)
					{
						if ((strcmp(stack[index_s - 1], tags_closing[j]) == 0) && (strcmp(stack[index_s - 2], tags_opening[j]) == 0))
						{
							memset(stack[index_s - 2], '\0', 100);
							memset(stack[index_s - 1], '\0', 100);
							index_s = index_s - 2;
							stack_size = stack_size - (strlen(tags_opening[j]) + strlen(tags_closing[j]));
							tags_len_frag = tags_len_frag - 2;
						}
					}
				}
			}
			//Not a block tag - put to output arr if there is some space
			if ((max_len - output_size) > (strlen(tag_title) + stack_size))
			{
				strcpy(output + output_size, tag_title);
				output_size = output_size + strlen(tag_title);
				output_arr_index = output_arr_index + strlen(tag_title);
				memset(tag_title, '\0', 500);
				tag_index = 0;
			}
			//Not block tag, no space - dump to html or stdout
			int tag_l = strlen(tag_title);
			if ((max_len - output_size) <= (tag_l + stack_size))
			{
				output_size = tag_closer(tags_len_frag, *stack, output, output_size, max_len, fragment_num);
				//processed = output_size;
				if (output_size <= 0) // error
				{
					printf("Failed to close tag for %d\n", fragment_num);
					return 0;
				}
				if (drop_buffer(fragment_num, tags_len_frag, *stack, output, output_size, max_len) == 0)
				{
					printf("Failed at %d\n", fragment_num);
					return 0;
				}
				//processed = 0;
				fragment_num++;
				memset(output, '\0', 4098);
				output_arr_index = 0;
				output_size = 0;
				if (tag_title[0] != '\0')
				{
					strcpy(output + output_size, tag_title);
					output_size = output_size + strlen(tag_title);
					output_arr_index = output_arr_index + strlen(tag_title);
					memset(tag_title, '\0', 500);
					tag_index = 0;
				}
				for (int i = 0; i < tags_len_frag; i++)
				{
					strcpy(output + output_size, stack[i]);
					output_size = output_size + strlen(stack[i]);
					output_arr_index = output_arr_index + strlen(stack[i]);
				}
				//processed = output_size;
			}
		}
		else if (tag_opened) // recording current tag name
		{
			tag_title[tag_index] = curr;
			tag_index++;
		}
		else if (!tag_opened) // if got past any tag (if it was in input file at all)
		{
			output[output_arr_index] = curr;
			output_arr_index++;
			output_size++;
			if ((max_len - output_size) <= stack_size)
			{
				output_size = tag_closer(tags_len_frag, *stack, output, output_size, max_len, fragment_num);
				//processed = output_size;
				if (output_size <= 0) // not splittable
				{
					printf("Failed to close tag for %d\n", fragment_num);
					return 0;
				}
				if (drop_buffer(fragment_num, tags_len_frag, *stack, output, output_size, max_len) == 0)
				{
					printf("Failed at %d\n", fragment_num);
					return 0;
				}
				//processed = 0;
				fragment_num++;
				memset(output, '\0', 4098);
				output_arr_index = 0;
				output_size = 0;
				if (tag_title[0] != '\0')
				{
					strcpy(output + output_size, tag_title);
					output_size = output_size + strlen(tag_title);
					output_arr_index = output_arr_index + strlen(tag_title);
					memset(tag_title, '\0', 500);
					tag_index = 0;
				}
				for (int i = 0; i < tags_len_frag; i++)
				{
					strcpy(output + output_size, stack[i]);
					output_size = output_size + strlen(stack[i]);
					output_arr_index = output_arr_index + strlen(stack[i]);
				}
			}
		}
	}
	return 1;
}
