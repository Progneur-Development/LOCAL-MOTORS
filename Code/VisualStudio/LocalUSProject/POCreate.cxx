#include "POCreate.hxx"

int col_item;
int col_pono;
int col_due_dt;
int col_qty_received;
int col_location;
FILE* succLogOut = stdout;
FILE* errLogOut = stdout;

int ITK_user_main(int argc, char *argv[])
{

	int iStatus					= ITK_ok;
	char *user_id				= NULL;
	char *user_pass				= NULL;
	char *user_group			= NULL;
	char *inputFilePath			= NULL;
	char *succLogFileName		= NULL;
	char *errLogFileName		= NULL;
	char *pszError				= NULL;
	succLogOut			= stdout;
	errLogOut				= stdout;
	std::vector<const char*> readDataVec ;

	user_id		= ITK_ask_cli_argument( "-u=" );
	user_pass	= ITK_ask_cli_argument( "-p=" );
	user_group	= ITK_ask_cli_argument( "-g=" );
	inputFilePath = ITK_ask_cli_argument( "-inputFilePath=" );
	TC_write_syslog("\n inside itk main");

	/* Display help if user asked for it */
	if (ITK_ask_cli_argument("-h") || argc == 1)
	{
		display_help();
		return EXIT_FAILURE;
	}

	succLogFileName = lm9_makeDir("SuccessLog");
	TC_write_syslog("succLogFileName==%s",succLogFileName);

	errLogFileName = lm9_makeDir("ErrorLog");
	TC_write_syslog("errLogFileName==%s",errLogFileName);

	//Generate logFile for code execution details
	if ((succLogFileName ) != 0)
	{
		if ((succLogOut = TC_fopen (succLogFileName, "w")) == 0)
		{
			TC_printf("ERROR : Unable to open log file %s\n", succLogFileName);
			display_help();
			exit(EXIT_FAILURE);
		}
		succLogOut = TC_fopen (succLogFileName, "w");
	} 	

	if ((errLogFileName ) != 0)
	{
		if ((errLogOut = TC_fopen (errLogFileName, "w")) == 0)
		{
			TC_printf("ERROR : Unable to open log file %s\n", errLogFileName);
			display_help();
			exit(EXIT_FAILURE);
		}
		errLogOut = TC_fopen (errLogFileName, "w");
	} 

	ITK_CALL(ITK_init_module(user_id, user_pass, user_group));
	if( iStatus == ITK_ok)  
	{    
		TC_write_syslog("\n Login successful");
		TC_fprintf(succLogOut, "\n Teamcenter login successful", "");

		//Get Preference Columns Data
		int prefCount=0;
		char** PrefereValues=NULL;
		iStatus=PREF_ask_char_values(Locals_US_Proj_Columns_PREF,&prefCount,&PrefereValues);
		TC_write_syslog("\n prefCount==%d",prefCount);

		if(prefCount<=0)
		{
			TC_fprintf(errLogOut, "\n Unable to import Preference list from teamcenter,\n Please contact you administrator","");
			exit(EXIT_FAILURE);
		}
		col_item = getPrefColumnId(ITEM_ID,prefCount,PrefereValues);
		col_pono = getPrefColumnId(PO_NUMBER,prefCount,PrefereValues);
		col_due_dt = getPrefColumnId(DUE_DATE,prefCount,PrefereValues);
		col_qty_received = getPrefColumnId(QUANTITY,prefCount,PrefereValues);
		col_location = getPrefColumnId(LOCATION,prefCount,PrefereValues);

		TC_write_syslog("\n col_item==%d  col_pono=%d col_due_dt=%d",col_item,col_pono,col_due_dt);

		TC_fprintf(succLogOut, "\n ------------Update PO Case-------------","");
		TC_fprintf(succLogOut, "\n Input file path for Update PO: %s", inputFilePath);
		iStatus=lm9_createUpdatePO(inputFilePath,succLogOut,errLogOut);
		if(PrefereValues!=NULL)
		{
			MEM_free(PrefereValues);
			prefCount=0;
		}
		//
	}else
	{
		TC_fprintf(succLogOut, "\n Teamcenter login failed", "");
	}

	return ITK_ok;
}

int lm9_createUpdatePO(char* fileName,FILE* succLogOut, FILE* errLogOut)
{
	int iStatus			= ITK_ok;
	int n_rows			= 0;
	tag_t tItem			= NULLTAG;
	tag_t tRevTag		= NULLTAG;
	tag_t *table_rows	= NULL;
	tag_t *newtable_rows= NULL;
	tag_t tOwner		= NULLTAG;
	char *ponumber		= NULL;
	char *inpPOnumber	= NULL;
	const char **LineData= NULL;
	date_t tDueDate,tOrderDate;
	std::vector<const char*> readDataVec ;
	std::vector<const char*> readRowVec ;
	int iCount=0;
	  std::ifstream in(fileName);
    if (in.fail()) 
		{
			printf("\n File not found");
			TC_fprintf(succLogOut, "\n Please provide the proper input CSV file");
			exit(EXIT_FAILURE);

	    }
    while(in.good())
    {
        std::vector<std::string> row = csv_read_row(in, ',');
		printf("\n");
		readDataVec.clear();
        for(int i=0, leng=row.size(); i<leng; i++)
        {
			const char *tok=NULL;
			tok=row[i].c_str();
			if (tok[0] == '$')
			{
				tok=remDollerSign(tok);
			}
			printf("\n tok==%s",tok);
			readDataVec.push_back(tok);
		}
  
		if(iCount==0)
		{
			iCount++;

		}
		else
		{
			TC_write_syslog("\n readDataVec==%d",readDataVec.size());

			char *itemIdStr = getSubStrItem(readDataVec[col_item],1,8);
			char *revStr = getSubStrItem(readDataVec[col_item],9,10);

			ITK_CALL(ITEM_find_item(itemIdStr,&tItem));

			TC_fprintf(errLogOut, "\n \n ----------------------------------------", "");
			TC_fprintf(succLogOut, "\n \n------------------------------------------", "");
			if(tItem!=NULLTAG)
			{
				inpPOnumber = NULL;

				TC_write_syslog("\n inpPOnumber===%d",inpPOnumber);

				ITK_CALL(ITEM_find_revision(tItem,revStr,&tRevTag));

				if(tRevTag!=NULLTAG)
				{
					char *revName   = NULL;
					char *PO_number = NULL;
					bool chkPOflag  = false;
					ITK_CALL(AOM_ask_table_rows(tRevTag,PO_RECORDS_TABLE,&n_rows,&newtable_rows));
					ITK_CALL(AOM_ask_value_string(tRevTag,OBJECT_STRING,&revName));

					TC_fprintf(succLogOut, "\n Item ID : %s  and  Item Revision : %s", itemIdStr,revName);

					if(readDataVec[col_pono]!=NULL)
					{
						inpPOnumber=(char*)malloc(strlen(readDataVec[col_pono])+1);
						strcpy(inpPOnumber,readDataVec[col_pono]);

						chkPOflag=checkPOExists(inpPOnumber,n_rows,newtable_rows);
						TC_write_syslog("\n chkPOflag ===%d",chkPOflag);
						if(chkPOflag)
						{
							bool chkdateFalg=false;

							TC_fprintf(succLogOut, "\n Po :%s is found to update table row ",inpPOnumber);
							TC_fprintf(succLogOut, "\n Checking for matching due date.... ","");
							for(int i=0;i<n_rows;i++)
							{
								date_t inpduedate_date1;
								date_t duedate_date1;
								date_t duedate_date2;
								char *strTcDate	= NULL;
								char *dDate_1	= NULL;
								char *dDate_2	= NULL;
								char *location	= NULL;
								char *dueCmpStr	= NULL;
								char *poNumber  = NULL;

								ITK_CALL(AOM_ask_value_string(newtable_rows[i],PO_NUMBER,&poNumber));
								ITK_CALL(AOM_ask_value_date(newtable_rows[i],DUE_DATE,&inpduedate_date1));
								ITK_date_to_string(inpduedate_date1,&strTcDate);
								//Requirement is to comapre only date(except hh:mm) so replaced Tc date hh:mm with 00:00
								dDate_1=strWith0TimeStamp(strTcDate);
								ITK_string_to_date(dDate_1,&duedate_date1);
								if(readDataVec[col_due_dt]!=NULL && poNumber!=NULL)
								{
									TC_write_syslog("\n ****dDate_1=%s",dDate_1);
									int  answer =0;		
									printf("\n readDataVec[col_due_dt]=%s",readDataVec[col_due_dt]);
									dueCmpStr = getFormattedDateStr(readDataVec[col_due_dt]);
									ITK_string_to_date(dueCmpStr,&duedate_date2);
									ITK_date_to_string(duedate_date2,&dDate_2);
									TC_write_syslog("\n dDate_1=%s  and dDate_2=%s",dDate_1,dDate_2);
									POM_compare_dates(duedate_date1,duedate_date2, &answer);
									if((strcmp(trim(inpPOnumber),trim(poNumber))==0) && answer==0)
									{
										int prefCnt		 = 0;
										char** PrefValues= NULL;
										chkdateFalg		 = true;
										int newQty		 = 0;
										int qtyInStock   = 0;

										TC_fprintf(succLogOut, "\n\n CSV input due date: %s and PO due date: %s are found same to update table row ",dDate_1,dDate_2);
										ITK_CALL(AOM_refresh(tRevTag,TRUE));
										ITK_CALL(AOM_refresh(newtable_rows[i],TRUE));

										//set values to table properties
										lm9_setRowData(newtable_rows[i],readDataVec,errLogOut,succLogOut,"UPDATE_PO");

										TC_write_syslog( "\n PO : %s is updated ",readDataVec[col_pono]);
										ITK_CALL(AOM_save(newtable_rows[i]));
										ITK_CALL(AOM_save(tRevTag));
										ITK_CALL(AOM_refresh(newtable_rows[i],FALSE));

										TC_fprintf(succLogOut, "\n Success: PO : %s is Updated for item revision :%s ",inpPOnumber,revName);

										//Update Quantity in stock property
										if(readDataVec[col_qty_received]!=NULL)
											newQty=atoi(readDataVec[col_qty_received]);
										ITK_CALL(AOM_refresh(tRevTag,TRUE));

										TC_write_syslog("\nreadDataVec[col_location]==='%s'",readDataVec[col_location]);
										if(readDataVec[col_location]!=NULL)
											location=chop(readDataVec[col_location]);

										if(location!=NULL && strcmp(location,Chandler_Microfactory)==0)
										{
											int qtyInStock=lm9_getQtyInStockVal(tRevTag,CURRENT_INV,newQty);
											ITK_CALL(AOM_set_value_int(tRevTag,CURRENT_INV,qtyInStock));
											TC_fprintf(succLogOut, "\n Quantity In Stock (Chandler) = %d propertie is updated for item revision :%s ",qtyInStock,revName);

										}else if(strcmp(location,Knoxville_Microfactory)==0)
										{
											int qtyInStock=lm9_getQtyInStockVal(tRevTag,CURRENT_INV_KNOX,newQty);
											ITK_CALL(AOM_set_value_int(tRevTag,CURRENT_INV_KNOX,qtyInStock));
											TC_fprintf(succLogOut, "\n Quantity In Stock (Knoxville)= %d propertie is updated for item revision :%s ",qtyInStock,revName);
										}else
										{
											TC_fprintf(errLogOut, "\n Quantity In Stock = %d propertie is not updated for item revision :%s.\n Please check for location column value is exist in csv or not",qtyInStock,revName);
										}

										ITK_CALL(AOM_save(tRevTag));
										ITK_CALL(AOM_refresh(tRevTag,FALSE));
									}else
									{
										//TC_fprintf(succLogOut, "\n **Note : CSV input due date: %s and PO due date: %s are not same to upadte table row ",dDate_1,dDate_2);
									}
								}
								else
								{
									TC_fprintf(errLogOut, "\n Po :%s is not updated for revision:%s because input due date: %s is not macthing. ",poNumber,revName,readDataVec[col_due_dt]);
									TC_write_syslog("\n Po :%s is not updated for revision:%s because input due date: %s is not macthing ");

								}
							}

							if(!chkdateFalg)
							{
								TC_fprintf(succLogOut, "\n Warning log : Indicate “PO %s Row with due date combination does NOT exist in Teamcenter, so creating it...” ",readDataVec[col_pono]);
								lm9_CreatePORow(errLogOut,succLogOut,tRevTag,readDataVec,inpPOnumber,revName);

							}
						}//chkPOflag if close
						else
						{
							TC_fprintf(succLogOut, "\n Warning log : Indicate “PO %s Row does NOT exist in Teamcenter, so creating it...” ",readDataVec[col_pono]);
							lm9_CreatePORow(errLogOut,succLogOut,tRevTag,readDataVec,inpPOnumber,revName);
						}
					}else
					{
						//Input PO number found NULL
						TC_fprintf(succLogOut, "\n Warning log : PO %s found NULL from csv input ",readDataVec[col_pono]);
					}


				}else
				{ 
					TC_fprintf(errLogOut, "\n Failuer: Indicate“LM Part Revision %s does NOT exist in Teamcenter”. ",revStr);
				}
			}
			else
			{
				TC_fprintf(errLogOut,"-----------------------------------------------------","");
				TC_fprintf(errLogOut, "\n Failure: Indicate“LM Part %s does NOT exist in Teamcenter to update PO : %s ", itemIdStr,readDataVec[col_pono]);
			}
			readDataVec.clear();
			iCount++;
		}
	}
	 in.close();
	return iStatus;	
}

//Calculate Quantity in stock value
int lm9_getQtyInStockVal(tag_t tRevTag,char* propName,int newQty)
{
	int iStatus=0;
	int oldQty=0;

	ITK_CALL(AOM_ask_value_int(tRevTag,propName,&oldQty));
	
	if(newQty==NULL)
		newQty=0;

	int qtyInStock=newQty+oldQty;

	TC_write_syslog("\n newQty===%d  + oldQty=%d = %d",newQty,oldQty,qtyInStock);
	TC_fprintf(succLogOut, "\n %d(Old Quantity In Stock) + %d(New Quantity Received)=%d ",oldQty,newQty,qtyInStock);

	return qtyInStock;

}

//
int lm9_setRowData(tag_t tableRowTag,std::vector<const char*> rowDataVec,FILE* errLogOut,FILE* succLogOut,char *funCallMode)
{
	int iStatus			 = 0;
	int prefCnt   	     = 0;
	int csv_col;
	char** PrefValues	 = NULL;
	std::vector<char*> prefArrData1;

	printf("\n inside lm9_setRowData");

	ITK_CALL(PREF_ask_char_values(Locals_US_Proj_Columns_PREF,&prefCnt,&PrefValues));
	printf("\n prefCnt==%d",prefCnt);

	if(prefCnt<=0)
	{
		TC_fprintf(errLogOut, "\n Unable to import Preference list from teamcenter,\n Please contact you administrator","");
		exit(EXIT_FAILURE);
	}

	for(int prefc=0;prefc<prefCnt;prefc++)
	{
		TC_write_syslog("\n PrefValues===%s",PrefValues[prefc]);
		prefArrData1.clear();
		if(strchr(PrefValues[prefc],':')!=NULL)
		{
			char *token1 = strtok(PrefValues[prefc], ":"); 
			TC_write_syslog("\n token1=%s",token1);
			prefArrData1.push_back(token1);
			while (token1 != NULL) 
			{ 
				token1 = strtok(NULL, ":"); 
				if (token1 != NULL) 
				{
					TC_write_syslog("\n token1=%s",token1);
					prefArrData1.push_back(token1);
				}

			} 

		}
		if(prefArrData1.size()>0)
		{
			const char* prop_name = prefArrData1[0];//property name
			const char* prop_type = prefArrData1[1];//data type
			int prop_colId		  = atoi(prefArrData1[2]);//csv col location

			printf("\n prop_name==%s  prop_type==%s prop_colId=%d",prop_name,prop_type,prop_colId);
			if(strcmp(ITEM_ID,prop_name)==0 )
			{
				TC_write_syslog("\n skipped %s property to update",prop_name);
			}else if(strcmp(STRING,prop_type)==0)
			{
				ITK_CALL(AOM_set_value_string(tableRowTag,prop_name,rowDataVec[prop_colId]));

			}else if(strcmp(INT,prop_type)==0)
			{
				int intVal=0;
				if(rowDataVec[prop_colId]!=NULL)
				{
					char *str=remComma(rowDataVec[prop_colId]);
					intVal=atoi(str);
				}
				printf("\n intVal===%d",intVal);
				ITK_CALL(AOM_set_value_int(tableRowTag,prop_name,intVal));//readDataVec[7]

			}else if(strcmp(DOUBLE,prop_type)==0) //get property data type
			{
				double dVal=0.0;
				printf("\n readRowdata[prop_colId]==%s",rowDataVec[prop_colId]);
				if(rowDataVec[prop_colId]!=NULL)
				{
					char *str=remComma(rowDataVec[prop_colId]);
					sscanf(str, "%lf", &dVal);
				}
				printf("\n dVal===%lf",dVal);
				ITK_CALL(AOM_set_value_double(tableRowTag,prop_name,dVal));//readDataVec[7]
			}else if(strcmp(DATE,prop_type)==0) 
			{
				date_t tDate_t;
				char *dateStr=NULL;
				if(rowDataVec[prop_colId]!=NULL)
					dateStr=getFormattedDateStr(rowDataVec[prop_colId]); 
				ITK_string_to_date(dateStr,&tDate_t);
				ITK_CALL(AOM_set_value_date(tableRowTag,prop_name,tDate_t));
			}
		}
	}
	if(PrefValues!=NULL)
	{
		MEM_free(PrefValues);
		prefCnt=0;
	}
	return iStatus;
}

int lm9_CreatePORow(FILE* errLogOut,FILE* succLogOut,tag_t tRevTag,std::vector<const char*> readRowdata,char *inpPOnumber,char *revName)
{
	int iStatus		     = 0;
	int  prefCnt		 = 0;
	tag_t *newtable_rows = NULL;


	ITK_CALL(AOM_refresh(tRevTag,TRUE));
	ITK_CALL(AOM_insert_table_rows(tRevTag,PO_RECORDS_TABLE,0,1,&newtable_rows));
	ITK_CALL(AOM_save(tRevTag));
	ITK_CALL(AOM_refresh(tRevTag,FALSE));


	ITK_CALL(AOM_refresh(tRevTag,TRUE));
	ITK_CALL(AOM_refresh(newtable_rows[0],TRUE));

	lm9_setRowData(newtable_rows[0],readRowdata,errLogOut,succLogOut,"CREATE_PO");


	ITK_CALL(AOM_save(newtable_rows[0]));
	ITK_CALL(AOM_save(tRevTag));
	ITK_CALL(AOM_refresh(newtable_rows[0],FALSE));
	ITK_CALL(AOM_refresh(tRevTag,FALSE));
	

	TC_fprintf(succLogOut, "\n PO : %s is created for item revision :%s ", inpPOnumber,revName);

	return iStatus;
}
static void display_help( void )
{
	TC_fprintf(stdout,"\n");
	TC_fprintf(stdout,"************* Local US UTILITY ************\n");
	TC_fprintf(stdout,"USAGE - ");
	TC_fprintf(stdout,"This utility Creates and Updates the table properties\n");
	TC_fprintf(stdout,"    -h    - Displays help\n");
	TC_fprintf(stdout,"Provide below input to run the utility.\n");
	TC_fprintf(stdout,"    -u					      - Tc user ID.\n");
	TC_fprintf(stdout,"    -p						  - Tc password associated with the -u=<user_id>. \n");
	TC_fprintf(stdout,"    -g						  - Tc group associated with the <user_id>.\n");
	TC_fprintf(stdout,"    -log_file                  - Log File full path.\n");
	TC_fprintf(stdout,"    -input_Create_PO_file      - input File full path, Create PO data.\n");
	TC_fprintf(stdout,"    -input_Update_PO_file      - input File full path, Update PO data.\n");
}

char* getSubStrItem(const char *itemstr,int position,int length)
{
	int c=0;
	char  sub[1000];

	while (c < length) {
		sub[c] = itemstr[position+c-1];
		c++;
	}
	sub[c] = '\0';

	TC_write_syslog("\n Required substring is \"%s\"\n", sub);
	return sub;
}

//Get Formatted date
char* getFormattedDateStr(const char* inDate)
{
	char* outputDate=NULL;

	vector< char*>  dChar;
	char *temp=NULL;
	char tempDate[50];

	strcpy(tempDate,inDate);

	printf("\n tempDate==%s",tempDate);
	printf("\n CSV input date: %s ",inDate);


	// split the mm/dd/yyyy using delim '/'
	if(strchr(inDate,'/')!=NULL)
	{
		char *token1 = strtok(tempDate, "/"); 
		while (token1 != NULL) 
		{ 
			//printf("%s\n", token1); 
			dChar.push_back(token1);
			token1 = strtok(NULL, "/"); 
		} 
	}


	outputDate=(char*)malloc(sizeof(char)* 100);
	strcpy(outputDate,""); //empty
	strcpy(outputDate,dChar[1]); //date
	strcat(outputDate,"-"); //-
	strcat(outputDate,getMonth(dChar[0])); //month
	strcat(outputDate,"-"); //-
	strcat(outputDate,dChar[2]); //year
	strcat(outputDate," "); //
	strcat(outputDate,"00"); //hours
	strcat(outputDate,":"); // :
	strcat(outputDate,"00"); //minute

	TC_write_syslog("\n Converted date in Teamcenter date format:%s ",outputDate);

	return outputDate;
}

// Returns the month from mm to MMM
// Returns the month from mm to MMM
static char* getMonth(char* month)
{
	if((strcmp(month,"01")==0) || (strcmp(month,"1")==0))
		return "Jan";
	if((strcmp(month,"02")==0) || (strcmp(month,"2")==0))
		return "Feb";
	if((strcmp(month,"03")==0) || (strcmp(month,"3")==0))
		return "Mar";
	if((strcmp(month,"04")==0) || (strcmp(month,"4")==0))
		return "Apr";
	if((strcmp(month,"05")==0) || (strcmp(month,"5")==0))
		return "May";
	if((strcmp(month,"06")==0) || (strcmp(month,"6")==0))
		return "Jun";
	if((strcmp(month,"07")==0) || (strcmp(month,"7")==0))
		return "Jul";
	if((strcmp(month,"08")==0) || (strcmp(month,"8")==0))
		return "Aug";
	if((strcmp(month,"09")==0) || (strcmp(month,"9")==0))
		return "Sep";
	if((strcmp(month,"10")==0) )
		return "Oct";
	if((strcmp(month,"11")==0) )
		return "Nov";
	if((strcmp(month,"12")==0) )
		return "Dec";
}
char *lm9_makeDir(char *logname)
{
	char* folderPath=NULL;

	int check; 
	char* dirname=NULL;
	char* dir_1 = "C:\\TEMP\\LOCAL_PROJLOG"; 
	char* currDateTime=NULL;

	//Get system current date
	ITK_ask_default_date_format(&currDateTime);
	TC_write_syslog("\n date=====%s", currDateTime);


	//DateFor folder creation
	char *currDate=getSubStrItem(currDateTime,1,11);
	currDate=strreplace(currDate,'-','_');
	currDate=chop(currDate);

	TC_write_syslog("\n currDate=====%s", currDate);
	// Replace date format from Wed Jun 10 16:10:32 2015 to Wed_Jun_10_16_10_32_2015
	currDateTime=strreplace(currDateTime,' ','_');
	currDateTime=strreplace(currDateTime,'-','_');
	currDateTime=strreplace(currDateTime,':','_');
	char *currentTime=chop(currDateTime);

	TC_write_syslog("\n currentTime:%s", currentTime);
	folderPath=(char*)malloc(sizeof(char)* 100);
	dirname=(char*)malloc(sizeof(char)* 100);
	strcpy(dirname,""); //empty
	strcat(dirname,dir_1);
	strcat(dirname,"\\");
	strcat(dirname,currDate);

	TC_write_syslog("\n dirname:%s", dirname);
	check = mkdir(dirname); 
	// check if directory is created or not 
	if (!check) 
	{   
		TC_write_syslog("Directory created\n"); 
		strcpy(folderPath,dirname);
		strcat(folderPath,"\\");
		strcat(folderPath,logname);
		strcat(folderPath,"_");
		strcat(folderPath,currentTime);
		strcat(folderPath,".txt");
	}
	else 
	{ 
		TC_write_syslog("\n ***Note : Please create 'C:\TEMP\LOCAL_PROJLOG' directory manually and run again exe/bat file"); 
		strcpy(folderPath,dirname);
		strcat(folderPath,"\\");
		strcat(folderPath,logname);
		strcat(folderPath,"_");
		strcat(folderPath,currentTime);
		strcat(folderPath,".txt");
		// exit(1); 
	} 
	return folderPath;
}
char* strreplace(char *str, char chr, char repl_chr)
{
	int i	= 0;
	char s[50];
	strcpy(s,str);
	while(s[i]!='\0')
	{
		if(s[i]==chr)
		{
			s[i]=repl_chr;
		}  
		i++; 
	}
	TC_write_syslog("strreplace==%s",s);
	return s;
}
// Function to remove all spaces from a given string 
char* trim(const char *str1) 
{ 
	int count = 0; 
	char *str=(char*)malloc(strlen(str1)+1);
	strcpy(str,str1);
	for (int i = 0; str[i]; i++) 
		if (str[i] != ' ') 
			str[count++] = str[i]; // here count is 
	str[count] = '\0'; 

	return str;
} 

/* removes the newline character from a string */
char* chop(const char *inp)
{
	char* s=(char *)malloc(strlen(inp)+1);
	strcpy(s,inp);
	s[strcspn ( s, "\n" )] = '\0';
	return s;
}

bool checkPOExists(char* inputPOumber,int n_rows,tag_t *newtable_rows)
{
	bool chkPOExists	= false;
	TC_write_syslog("\n inputPOumber==%s   ",inputPOumber);
	for(int i=0;i<n_rows;i++)
	{
		char* poNumber=NULL;
		//	TC_write_syslog("\n inputPOumber==%s   ",inputPOumber);

		ITK_CALL(AOM_ask_value_string(newtable_rows[i],"lm9_PONumber",&poNumber));
		TC_write_syslog("\n inputPOumber==%s  poNumber=%s ",inputPOumber,poNumber);
		if(strcmp(trim(inputPOumber),trim(poNumber))==0)
		{
			chkPOExists=true;
			break;
		}
	}
	return chkPOExists;
}

char* remDollerSign(const char *contents)
{
	char *p = (char*)malloc( sizeof(*p) * strlen(contents) );
	int i;
	for(i=0; i<strlen(contents); i++)
	{
		p[i]=contents[i+1];
	}
	return p;
}
int getPrefColumnId(char *prop_id_str,int prefCnt,char **PrefValues)
{
	int iStatus   =0;
	int csv_col;
	std::vector<char*> prefArrData;

	TC_write_syslog("\n prop_id_str==%s",prop_id_str);

	for(int prefc=0;prefc<prefCnt;prefc++)
	{
		TC_write_syslog("\n PrefValues===%s",PrefValues[prefc]);
		prefArrData.clear();
		if(strchr(PrefValues[prefc],':')!=NULL)
		{
			char *token1 = strtok(PrefValues[prefc], ":"); 
			prefArrData.push_back(token1);
			while (token1 != NULL) 
			{ 
				token1 = strtok(NULL, ":"); 
				if (token1 != NULL) 
				{
					prefArrData.push_back(token1);
				}
			} 
		}
		//check item id
		if(prefArrData.size()>0 && strcmp(prop_id_str,prefArrData[0])==0)
		{
			csv_col=atoi(prefArrData[2]);
			break;
		}
	}
	return csv_col;
}


char* remComma(const char *inpStr_1)
{
	char *inpStr=(char *)malloc(strlen(inpStr_1)+1);
	TC_write_syslog("\n before rem  str===%s",inpStr_1);
	strcpy(inpStr,inpStr_1);
	int count = 0; 
	for (int i = 0; inpStr[i]; i++) 
		if (inpStr[i] != ',') 
			inpStr[count++] = inpStr[i]; // here count is 
	// incremented 
	inpStr[count] = '\0'; 

	TC_write_syslog("\nafter  str===%s",inpStr);
	return inpStr;
}

std::vector<std::string> csv_read_row(std::istream &in, char delimiter)
{
    std::stringstream ss;
    bool inquotes = false;
    std::vector<std::string> row;//relying on RVO
    while(in.good())
    {
        char c = in.get();
        if (!inquotes && c=='"') //beginquotechar
        {
            inquotes=true;
        }
        else if (inquotes && c=='"') //quotechar
        {
            if ( in.peek() == '"')//2 consecutive quotes resolve to 1
            {
                ss << (char)in.get();
            }
            else //endquotechar
            {
                inquotes=false;
            }
        }
        else if (!inquotes && c==delimiter) //end of field
        {
            row.push_back( ss.str() );
            ss.str("");
        }
        else if (!inquotes && (c=='\r' || c=='\n') )
        {
            if(in.peek()=='\n') { in.get(); }
            row.push_back( ss.str() );
            return row;
        }
        else
        {
            ss << c;
        }
    }
}

std::vector<std::string> csv_read_row(std::string &line, char delimiter)
{
    std::stringstream ss(line);
    return csv_read_row(ss, delimiter);
}

//Replaced existing date's hh:mm with 00:00
char* strWith0TimeStamp(char *dDate_1)
{
	int answer=0;
	char *dateStr1=NULL;
	TC_write_syslog("\n Before split dDate_1==%s",dDate_1);
	
	dDate_1 = getSubStrItem(dDate_1,1,12);
	TC_write_syslog("\n after split dDate_1==%s",dDate_1);
	dateStr1=(char *)malloc(strlen(dDate_1)+1);
	strcpy(dateStr1,dDate_1);
	strcat(dateStr1,"00"); //hours
	strcat(dateStr1,":"); // :
	strcat(dateStr1,"00"); //minute

	TC_write_syslog("\n New date=%s",dateStr1);
	return dateStr1;
}