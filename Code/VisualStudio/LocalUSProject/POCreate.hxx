#include<conio.h>
#include<stdio.h>
#include<tc\tc.h>
#include<sa\user.h>
#include<tccore\item.h>
#include<bom\bom.h>
#include<string.h>
#include<tccore\workspaceobject.h>
#include<tccore\aom_prop.h>
#include<tccore\aom.h>
#include<sa\user.h>
#include<tc\emh.h>
#include<fclasses\tc_string.h>
#include<stdlib.h>
#include <tcinit/tcinit.h>
#include <tccore/item_errors.h>
#include <tc/tc.h>
#include <tc/preferences.h>
#include <tc/tc_util.h>
#include<vector>
#include<map>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <iterator> 
#include <tc/Folder.h>
#include <tc/LoggedInUser.hxx>
#include <base_utils/TcResultStatus.hxx>
#include <base_utils/IFail.hxx>
#include <direct.h>
#include <process.h> 
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include<dirent.h>
#include <assert.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <istream>

using namespace std;
using std::cout;
using std::endl;

#define EXIT_FAILURE 1
#define ITK_CALL(x) {           \
    int stat;                     \
    char *err_string;             \
    if( (stat = (x)) != ITK_ok)   \
    {                             \
		EMH_get_error_string (NULLTAG, stat, &err_string);                 \
		printf ("ERROR: %d ERROR MSG: %s.\n", stat, err_string);           \
		printf ("FUNCTION: %s\nFILE: %s LINE: %d\n",#x, __FILE__, __LINE__); \
		if(err_string) MEM_free(err_string);                                \
		exit (EXIT_FAILURE);                                                   \
    }                                                                    \
}

#define Locals_US_Proj_Columns_PREF "Locals_US_Proj_Columns"
#define ITEM_ID "item_id"
#define PO_NUMBER "lm9_PONumber"
#define DUE_DATE "lm9_DueDate"
#define QUANTITY "lm9_Quantity"
#define LOCATION "lm9_Location"
#define PROJECT "lm9_Project"
#define PO_RECORDS_TABLE "lm9_PORecordsTable"
#define OBJECT_STRING "object_string"
#define CURRENT_INV "lm9_CurrentInv"
#define CURRENT_INV_KNOX "lm9_CurrentInv_KNOX"
#define Knoxville_Microfactory "Knoxville Microfactory"
#define Chandler_Microfactory "Chandler Microfactory"

#define STRING "string"
#define INT "int"
#define DOUBLE "double"
#define DATE "date"

int lm9_createUpdatePO(char* inUpdateFilePath,FILE* succLogOut, FILE* errLogOut);
int lm9_setRowData(tag_t tableRowTag,std::vector<const char*> rowDataVec,FILE* errLogOut,FILE* succLogOut,char *funCallMode);
int lm9_CreatePORow(FILE* errLogOut,FILE* succLogOut,tag_t tRevTag,std::vector<const char*> readRowdata,char *inpPOnumber,char *revName);
static void display_help( void );
char* getSubStrItem(const char *itemstr,int position,int length);
char* getFormattedDateStr(const char* inDate);
static char* getMonth(char* month);
char *lm9_makeDir(char *logFile);
char* strreplace(char *str, char chr, char repl_chr);
char* trim(const char *str1);
char* chop(const char *string);
bool checkPOExists(char* inputPOumber,int n_rows,tag_t *newtable_rows);
char* remDollerSign(const char *contents);
int getPrefColumnId(char *prop_id_str,int prefCnt,char **PrefValues);
char* remComma(const char *inpStr_1);
std::vector<std::string> csv_read_row(std::istream &in, char delimiter);
std::vector<std::string> csv_read_row(std::string &line, char delimiter);
int lm9_getQtyInStockVal(tag_t tRevTag,char* propName,int newQty);
char* strWith0TimeStamp(char *dDate_1);


