#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#define MAX_LINE_LENGTH 100

int main(int argc, char **argv)
{
    FILE *f_in, *f_out;
    char *name_ptr, *ptr;
    off_t i;
    struct stat st;
    char out_name[FILENAME_MAX];
    unsigned char png_byte, last_byte_octal, output_length;

    if (argc != 2)
    {
        fprintf(stderr, "this requires an input file\n");
        return -1;
    }

    f_in = fopen(argv[1], "rb");
    if (!f_in)
    {
        fprintf(stderr, "this requires the input file to exist\n");
        return -1;
    }

    if (stat(argv[1], &st))
    {
        fprintf(stderr, "this requires the input file to have no problems\n");
        return -1;
    }

    name_ptr = strrchr(argv[1], '/'); // *nix path
    if (!name_ptr)
        name_ptr = strrchr(argv[1], '\\'); // Wind*ws path
    if (name_ptr)
        name_ptr++;
    else
        name_ptr = argv[1];

    if (!*name_ptr || strlen(name_ptr) >= FILENAME_MAX)
    {
        fprintf(stderr, "this requires a reasonable length for the file name\n");
        return -1;
    }
    strcpy(out_name, name_ptr);

    ptr = out_name;
    while (*ptr)
    {
        *ptr = tolower(*ptr);
        ptr++;
    }

    ptr = strrchr(out_name, '.');
    if (*ptr)
        strcpy(ptr, ".h");
    else
        strcat(out_name, ".h");

    f_out = fopen(out_name, "w");
    if (!f_out)
    {
        fprintf(stderr, "this requires the output file to be created\n");
        return -1;
    }

    fprintf(stderr, "creating %s, please hold on.\n", out_name);

    ptr = out_name;
    while (*ptr)
    {
        if (*ptr == '.')
            *ptr = '_';
        ptr++;
    }

    for (i = 0; i < 2; i++)
    {
        fprintf(f_out, "#%s", !i ? "ifndef" : "define");
        fprintf(f_out, " _");
        ptr = out_name;
        while (*ptr)
        {
            fprintf(f_out, "%c", toupper(*ptr));
            ptr++;
        }
        fprintf(f_out, "_\n");
    }

    fprintf(f_out, "\n"
                   "static const struct {\n"
                   "\tsize_t size;\n"
                   "\tunsigned char data [%lu];\n"
                   "} ",
            (unsigned long)st.st_size);

    ptr = name_ptr;
    while (*ptr)
    {
        if (*ptr == '.')
            fprintf(f_out, "_");
        else
            fprintf(f_out, "%c", tolower(*ptr));
        ptr++;
    }

    fprintf(f_out, " = {\n"
                   "\t%lu,\n\"",
            (unsigned long)st.st_size);

    last_byte_octal = 0;
    output_length = 1;
    for (i = 0; i < st.st_size; i++)
    {
        png_byte = fgetc(f_in);
        if (png_byte == '\\')
        {
            output_length += 2;
            if (output_length >= MAX_LINE_LENGTH)
            {
                fprintf(f_out, "\"\n\"");
                output_length = 3;
            }
            fprintf(f_out, "\\\\");
            last_byte_octal = 0;
        }
        else if (png_byte == 9)
        {
            output_length += 2;
            if (output_length >= MAX_LINE_LENGTH)
            {
                fprintf(f_out, "\"\n\"");
                output_length = 3;
            }
            fprintf(f_out, "\\t");
            last_byte_octal = 0;
        }
        else if (png_byte < ' ' || png_byte == '\"')
        {
            output_length += (png_byte < 8) ? 2 : 3;
            last_byte_octal = 1;
            if (output_length >= MAX_LINE_LENGTH)
            {
                fprintf(f_out, "\"\n\"");
                output_length = (png_byte < 8) ? 3 : 4;
            }
            fprintf(f_out, "\\%o", png_byte);
        }
        else if (png_byte > '~')
        {
            output_length += 4;
            if (output_length >= MAX_LINE_LENGTH)
            {
                fprintf(f_out, "\"\n\"");
                output_length = 5;
            }
            fprintf(f_out, "\\x%X", png_byte);
            last_byte_octal = 1;
        }
        else
        {
            output_length += (last_byte_octal && isxdigit(png_byte)) ? 3 : 1;
            if (output_length >= MAX_LINE_LENGTH)
            {
                fprintf(f_out, "\"\n\"");
                output_length = 2;
                last_byte_octal = 0;
            }
            if (last_byte_octal && isxdigit(png_byte))
                fprintf(f_out, "\"\"");
            fprintf(f_out, "%c", png_byte);
            last_byte_octal = 0;
        }
    }
    fprintf(f_out, "\"\n};\n\n#endif\n");

    fclose(f_in);
    fclose(f_out);

    fprintf(stderr, "done.\n");

    return 0;
}