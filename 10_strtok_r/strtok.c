#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    char* mutable_string;
    const char* delim;
} strtok_t;

char* strtok_r(strtok_t* detoken);

// check
static bool is_delimiter(const char ch, const char* delim_list)
{
    uint32_t index = 0;
    while(ch != delim_list[index])
    {
        index++;

        if(delim_list[index] == '\0')   // delimiter list ended
            return false;
    }
    return true;
}

// return the ref character
// update the detoken->track_ptr for subsequent invocations 

char* strtok_r(strtok_t* detoken)
{
    char ch     = '\0';
    char* start = NULL;       // return pointer
    char* end   = NULL;       // end of first token - where existing delim will be replaced by \0

    if(!detoken)
        return NULL;

    start = detoken->mutable_string;
    ch = *start;

    // check if it's end of string OR a NUL string itself
    if(ch == '\0')
    {
        return NULL;
    }

    // Step 1: Look for the first non-delim char - If found NONE, NULL will be returned

    while(is_delimiter(ch, detoken->delim))     // terminating condition is the hitting of last \0 of the string
    {
        start++;
        ch = *start;
    }

    if(ch == '\0')
    {
        detoken->mutable_string = start;
        return NULL;    
    }

    // Step 2: if non-delim char found, proceed to find the first delim char and replace it with \0

    end = start;
    end++;
    ch = *end;
    while(!is_delimiter(ch, detoken->delim))
    {
        end++;
        ch = *end;
    }

    if(ch == '\0')
    {
        detoken->mutable_string = end;
        return start;
    } 

    detoken->mutable_string = end++;
    return start;
    
}

int main()
{
    strtok_t token_1;
    char ip[] = ":::192:::168:2:1:::"; 
    token_1.mutable_string = ip;
    token_1.delim = ":";

    printf("%s\n",strtok_r(&token_1));

    return 0;
}

