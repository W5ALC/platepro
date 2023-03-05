#pragma once
/* This file is a part of platepro program, which is GNU GPLv2 licensed */
#include <vector>
#include <map>
#include <set>
#include <string>
#include "fileio.h"

#define PDF_NAME_MAX_LEN 255
#define PDF_ID_MAX_LEN 255
#define XREF_ENT_LEN 18// [10 digit obj no]<space>[5 digit gen no]<space>[f or n]
#define LLEN 256
#define STARTXREF_OFFSET 64 // how much to seek from end to read startxref

/*
  PDF includes eight basic types of objects: Boolean values, Integer and Real numbers,
  Strings, Names, Arrays, Dictionaries, Streams, and the null object.
  Objects may be labelled so that they can be referred to by other objects. A labelled
  object is called an indirect object.
*/
class Token;
class PdfObject;
class ObjectTable;

typedef struct {
    char *data;
    int len;// string length excluding null character
} String;

enum {
    BYTE_STR,
    HEX_STR
};

typedef bool    BoolObj;// keyword 'true' and 'false'
typedef int     IntObj;
typedef double  RealObj;// eg. 2.0, 0.2, 2., .2, +2.0, -2.0, -2., -.2 etc
typedef char*   NameObj; // name starting with '/' , eg - /Page , /Count
typedef String  StringObj; // (abcd) or <eaffbb00>

std::string pdfstr2bytes(String str, int *str_type);
void        bytes2pdfstr(std::string str, String &out_str, int str_type);


typedef std::vector<PdfObject*>::iterator ArrayIter;

class ArrayObj
{
public:
    std::vector<PdfObject*> array;

    int         count();
    PdfObject*  at (int index);
    void        append (PdfObject *item);
    void        deleteItems();
    int         write (FILE *f);
    //allows range based for-loop
    ArrayIter   begin();
    ArrayIter   end();
};

typedef std::map<std::string, PdfObject*>::iterator MapIter;
// filter class used to remove unnecessary items in dict
typedef std::set<std::string> DictFilter;

class DictObj
{
public:
    std::map<std::string, PdfObject*> dict;

    bool        contains (std::string key);
    PdfObject*  get (std::string key);
    void        add (std::string key, PdfObject *val);
    PdfObject*  newItem (std::string key);// if val exist, clear and return obj, else create new PdfObject
    void        deleteItem (std::string key);
    void        deleteItems();
    void        setDict (std::map<std::string, PdfObject*> &map);
    void        merge (DictObj *src_dict);// hard copy new items, overwrite old items
    void        filter (DictFilter &filter_set);// remove all objects which are not in filter_set
    int         write (FILE *f);
    MapIter     begin();
    MapIter     end();
    PdfObject* operator[] (std::string key);
};


class StreamObj
{
public:
    size_t begin;//pos where stream begins in file
    size_t len;
    bool decompressed;
    DictObj dict;
    char *stream;
    int write(FILE *f);
    bool decompress();
    bool compress (const char *filter);

    StreamObj();
    ~StreamObj();
};

typedef struct {
    int major;
    int minor;
    PdfObject *obj;
} IndirectObj;



typedef enum {
    PDF_OBJ_BOOL, PDF_OBJ_INT, PDF_OBJ_REAL, PDF_OBJ_STR,
    PDF_OBJ_NAME, PDF_OBJ_ARRAY, PDF_OBJ_DICT, PDF_OBJ_STREAM,
    PDF_OBJ_INDIRECT, PDF_OBJ_INDIRECT_REF, PDF_OBJ_NULL, PDF_OBJ_UNKNOWN
} ObjectType;

// can't store objects inside union that contain std::map, vector (eg. DictObj).
// So storing pointers of these objects inside union.

class PdfObject
{
public:
    ObjectType type;
    union {
        BoolObj     boolean;
        IntObj      integer;
        RealObj     real;
        StringObj   str;
        NameObj     name;
        DictObj     *dict;// always allocates dict
        ArrayObj    *array;// always allocates array
        StreamObj   *stream;// always allocates stream, may allocate stream->stream
        IndirectObj indirect;// always allocates indirect.obj if PDF_OBJ_INDIRECT
    };
    PdfObject();
    void setType(ObjectType obj_type);
    bool read (MYFILE *f, ObjectTable *xref, Token *last_tok);
    bool readFromString (const char *str);
    int write (FILE *f);
    int copyFrom (PdfObject *src_obj);
    void clear();
    ~PdfObject();
};
/* iterate array of pointers like this ...
    for (auto item = obj->array->begin(); item != obj->array->end(); item++) {
        val = (*item)->integer;
    }
*/

enum {
    XREF_INVALID,
    XREF_TABLE,
    XREF_STREAM
};

enum {
    FREE_OBJ,
    NONFREE_OBJ,
    COMPRESSED_OBJ,
};

int getXrefType(MYFILE *f);

// always obj==NULL for free obj, and never NULL for nonfree obj
typedef struct {
    PdfObject *obj;
    int8_t type;  // free(f), nonfree(n), compressed
    int major;   // object no.
    int minor; // gen id for type 1, (always 0 for type 2)
    union {
        int offset; // offset of obj from beginning of file (type 1 only)
        int next_free;// obj no of next free obj (type 0 only)
        int obj_stm; // obj no. of object stream where obj is stored (type 2 only)
    };
    int index;// index no. within the obj stream (for type 2)
    bool used;
} ObjectTableItem;


/* If there is any error in object table, it should be fixed while reading the table. */

class ObjectTable
{
public:
    std::vector<ObjectTableItem> table;

    int count();
    void expandToFit(size_t size);
    int addObject (PdfObject *obj);
    PdfObject* getObject(int major, int minor);
    bool read (MYFILE *f, size_t xref_pos);
    bool read (PdfObject *stream, PdfObject *p_trailer);
    bool readObject(MYFILE *f, int major);
    void readObjects(MYFILE *f);
    void writeObjects(FILE *f);
    void writeXref (FILE *f);

    ObjectTableItem& operator[] (int index);
};


/* constants defining whitespace characters */
enum  { CHAR_NULL=0, CHAR_TAB=9, CHAR_LF=10, CHAR_FF=12, CHAR_CR=13, CHAR_SP=32};

/* constants identifying the token type */
typedef enum {
    TOK_INT, TOK_REAL, TOK_STR, TOK_NAME, TOK_BDICT, TOK_EDICT,
    TOK_BARRAY, TOK_EARRAY, TOK_ID, TOK_EOF, TOK_UNKNOWN
} TokType;

class Token
{
public:
    TokType type;
    double  real;
    int     integer;
    char    name[PDF_NAME_MAX_LEN];
    char    id[PDF_ID_MAX_LEN];
    String  str;
    bool    new_line;//if there is newline before parsed token
    int     sign;// 1='+', -1='-', 0=no sign

    Token();
    bool get(MYFILE *f);
    void freeData();
};

#define isInt(obj) (((obj)!=NULL) && ((obj)->type==PDF_OBJ_INT))
#define isReal(obj) (((obj)!=NULL) && ((obj)->type==PDF_OBJ_REAL))
#define isName(obj) (((obj)!=NULL) && ((obj)->type==PDF_OBJ_NAME))
#define isString(obj) (((obj)!=NULL) && ((obj)->type==PDF_OBJ_STR))
#define isArray(obj) (((obj)!=NULL) && ((obj)->type==PDF_OBJ_ARRAY))
#define isDict(obj) (((obj)!=NULL) && ((obj)->type==PDF_OBJ_DICT))
#define isStream(obj) (((obj)!=NULL) && ((obj)->type==PDF_OBJ_STREAM))
#define isRef(obj) (((obj)!=NULL) && ((obj)->type==PDF_OBJ_INDIRECT_REF))

// dereferencing a pdf object
inline PdfObject* derefObject(PdfObject *obj, ObjectTable &obj_table)
{
    while (isRef(obj)){
        obj = obj_table.getObject(obj->indirect.major, obj->indirect.minor);
    }
    return obj;
}
