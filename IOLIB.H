extern char 	**ReadHeader();
extern void	DisplayHeader();
extern char	*GetHeaderParameter();

#define MAXLINE 256
/*
** this is the magic start of header string
*/
#define MAGIC_SOH_STR "%%BEGINHEADER"
/*
** this is the magic end of header string
*/
#define MAGIC_EOH_STR "%%ENDHEADER"
/*
** this is the length of the magic start of header string %%BEGINHEADER
*/
#define MAGIC_SOH_STRSIZE	14
/*
** this is the length of the magic end of header string %%ENDHEADER
*/
#define MAGIC_EOH_STRSIZE	12

#define ASCII	0
#define BINARY	1

typedef struct field_info_type {
    char	*name;	
    int		column;	
    int		type;
    int		size;
    int		count;
} FieldInfo;

