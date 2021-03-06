/******************************************************************************/
/* Modules:
/*      Main                Main driver loop, executes queries.
/*      SaveResults         Save the results of the query in html file
/*      DisplayResults      Display the results of the query if any
/*      AllocateBindings    Bind column data
/*      DisplayTitles       Print column titles
/*      SetConsole          Set console display mode
/*      HandleError         Show ODBC error messages
/******************************************************************************/

#include <windows.h>
#include <format>
#include <sql.h>
#include <sqlext.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <stdlib.h>
#include <sal.h>
#include <fstream>
#include <time.h>
#include <string>


/*******************************************/
/* Macro to call ODBC functions and        */
/* report an error on failure.             */
/* Takes handle, handle type, and stmt     */
/*******************************************/

#define TRYODBC(h, ht, x)   {   RETCODE rc = x;\
                                if (rc != SQL_SUCCESS) \
                                { \
                                    HandleDiagnosticRecord (h, ht, rc); \
                                } \
                                if (rc == SQL_ERROR) \
                                { \
                                    fwprintf(stderr, L"Error in " L#x L"\n"); \
                                    goto Exit;  \
                                }  \
                            }
/******************************************/
/* Structure to store information about   */
/* a column.
/******************************************/

typedef struct STR_BINDING {
    SQLSMALLINT         cDisplaySize;           /* size to display  */
    WCHAR*              wszBuffer;              /* display buffer   */
    SQLLEN              indPtr;                 /* size or null     */
    BOOL                fChar;                  /* character col?   */
    struct STR_BINDING* sNext;                  /* linked list      */
} BINDING;

HANDLE hConsole;

/******************************************/
/* Forward references                     */
/******************************************/

void HandleDiagnosticRecord(SQLHANDLE      hHandle,
    SQLSMALLINT    hType,
    RETCODE        RetCode);

void SaveResults(HSTMT         hStmt,
    SQLSMALLINT cCols);

void DisplayResults(HSTMT       hStmt,
    SQLSMALLINT cCols);

void AllocateBindings(HSTMT         hStmt,
    SQLSMALLINT   cCols,
    BINDING** ppBinding,
    SQLSMALLINT* pDisplay,
    bool isFile);

void DisplayTitles(HSTMT    hStmt,
    DWORD    cDisplaySize,
    BINDING* pBinding);

void SetConsole(DWORD   cDisplaySize,
    BOOL    fInvert);

/*****************************************/
/* Some constants                        */
/*****************************************/


#define DISPLAY_MAX 35          // Arbitrary limit on column width to display
#define DISPLAY_FORMAT_EXTRA 3  // Per column extra display bytes (| <data> )
#define DISPLAY_FORMAT      L"%c %*.*s "
#define DISPLAY_FORMAT_C    L"%c %-*.*s "
#define NULL_SIZE           6   // <NULL>
#define SQL_QUERY_SIZE      1000 // Max. Num characters for SQL Query passed in.

#define PIPE                L'|'

SHORT   gHeight = 80;       // Users screen height


int __cdecl getTable(SQLWCHAR* wszInput, bool createFile)
{
    SQLHENV     hEnv = NULL;
    SQLHDBC     hDbc = NULL;
    SQLHSTMT    hStmt = NULL;
    WCHAR* pwszConnStr;
    // Allocate an environment

    if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv) == SQL_ERROR)
    {
        fwprintf(stderr, L"Unable to allocate an environment handle\n");
        exit(-1);
    }

    // Register this as an application that expects 3.x behavior,
    // you must register something if you use AllocHandle

    TRYODBC(hEnv,
        SQL_HANDLE_ENV,
        SQLSetEnvAttr(hEnv,
            SQL_ATTR_ODBC_VERSION,
            (SQLPOINTER)SQL_OV_ODBC3,
            0));

    // Allocate a connection
    TRYODBC(hEnv,
        SQL_HANDLE_ENV,
        SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc));

        pwszConnStr = (WCHAR*)L"Dsn=MySQLDB;uid=root;server=127.0.0.1;database=senderiondb;port=3306";

    // Connect to the driver.  Use the connection string if supplied
    // on the input, otherwise let the driver manager prompt for input.

    TRYODBC(hDbc,
        SQL_HANDLE_DBC,
        SQLDriverConnect(hDbc,
            GetDesktopWindow(),
            pwszConnStr,
            SQL_NTS,
            NULL,
            0,
            NULL,
            SQL_DRIVER_COMPLETE));

    TRYODBC(hDbc,
        SQL_HANDLE_DBC,
        SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt));


        // Loop to get input and execute queries

        RETCODE     RetCode;
        SQLSMALLINT sNumResults;

        // Execute the query

        RetCode = SQLExecDirect(hStmt, wszInput, SQL_NTS);

        switch (RetCode)
        {
        case SQL_SUCCESS_WITH_INFO:
        {
            HandleDiagnosticRecord(hStmt, SQL_HANDLE_STMT, RetCode);
            // fall through
        }
        case SQL_SUCCESS:
        {
            // If this is a row-returning query, display
            // results
            TRYODBC(hStmt,
                SQL_HANDLE_STMT,
                SQLNumResultCols(hStmt, &sNumResults));

            if (sNumResults > 0)
            {
                if (createFile) {
                    SaveResults(hStmt, sNumResults);
                }
                else {
                    DisplayResults(hStmt, sNumResults);
                }
            }
            else
            {
                SQLLEN cRowCount;

                TRYODBC(hStmt,
                    SQL_HANDLE_STMT,
                    SQLRowCount(hStmt, &cRowCount));

                if (cRowCount >= 0)
                {
                    
                }
            }
            break;
        }

        case SQL_ERROR:
        {
            HandleDiagnosticRecord(hStmt, SQL_HANDLE_STMT, RetCode);
            break;
        }

        default:
            fwprintf(stderr, L"Unexpected return code %hd!\n", RetCode);

        }
        TRYODBC(hStmt,
            SQL_HANDLE_STMT,
            SQLFreeStmt(hStmt, SQL_CLOSE));


Exit:

    // Free ODBC handles and exit

    if (hStmt)
    {
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    }

    if (hDbc)
    {
        SQLDisconnect(hDbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
    }

    if (hEnv)
    {
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
    }

    return 0;

}

/************************************************************************
/* DisplayResults: display results of a select query
/*
/* Parameters:
/*      hStmt      ODBC statement handle
/*      cCols      Count of columns
/************************************************************************/

void SaveResults(HSTMT         hStmt,
    SQLSMALLINT cCols)
{
    BINDING* pFirstBinding, * pThisBinding;
    SQLSMALLINT     cDisplaySize;
    WCHAR           wszTitle[DISPLAY_MAX];
    SQLSMALLINT     iCol = 1;
    RETCODE         RetCode = SQL_SUCCESS;
    int             iCount = 0;
    std::wofstream   File;
    int             FileName = time(NULL);
    char            name[50];
    bool            fNoData = false;
    setlocale(LC_ALL, "rus_rus.866");
    sprintf_s(name, sizeof(name), "%d.html", FileName);
    File.open(name);
    File << L"<!Doctype html>\n<html><head><meta charset=\"utf-8\" /><title>Report</title></head><body>\n";
    AllocateBindings(hStmt, cCols, &pFirstBinding, &cDisplaySize, true);
    File << L"<table>\n";
    File << L"\t<tr>\n";
    /* ------------- Titles ------------*/
    for (pThisBinding = pFirstBinding;
        pThisBinding;
        pThisBinding = pThisBinding->sNext)
    {
        TRYODBC(hStmt,
            SQL_HANDLE_STMT,
            SQLColAttribute(hStmt,
                iCol++,
                SQL_DESC_NAME,
                wszTitle,
                sizeof(wszTitle), // Note count of bytes!
                NULL,
                NULL));
        File << L"\t\t<th style=\"border: 1px solid gray;\">";
        File << wszTitle;
        File << L"  </th>\n";
    }
Exit: {
File << L"\t</tr>\n";
    }
/* ------------- Content ------------*/
std::locale::global(std::locale("ru_RU.UTF-8"));
File.imbue(std::locale());
do {
    TRYODBC(hStmt, SQL_HANDLE_STMT, RetCode = SQLFetch(hStmt));
    if (RetCode == SQL_NO_DATA_FOUND)
    {
        fNoData = true;
    }
    else
    {
        File << L"\t<tr>\n";
        for (pThisBinding = pFirstBinding;
            pThisBinding;
            pThisBinding = pThisBinding->sNext)
        {
            
            if (pThisBinding->indPtr != SQL_NULL_DATA)
            {
                File << L"\t\t<td style=\"border: 1px solid gray;\">";
                File << pThisBinding->wszBuffer;
                File << L"</td>\n";
            }
            else
            {
                File << L"\t\t<td>NULL</td>\n";
            }
            
        }
        File << L"\t</tr>\n";
        
    }
} while (!fNoData);

File << L"</table></body></html>";
File.close();
wprintf(L"Report created with name %d.html\n", FileName);
wprintf(L"Press ESC to Quit");

}

/************************************************************************
/* DisplayResults: display results of a select query
/*
/* Parameters:
/*      hStmt      ODBC statement handle
/*      cCols      Count of columns
/************************************************************************/

void DisplayResults(HSTMT       hStmt,
    SQLSMALLINT cCols)
{
    BINDING* pFirstBinding, * pThisBinding;
    SQLSMALLINT     cDisplaySize;
    RETCODE         RetCode = SQL_SUCCESS;
    int             iCount = 0;

    // Allocate memory for each column 

    AllocateBindings(hStmt, cCols, &pFirstBinding, &cDisplaySize, false);

    // Set the display mode and write the titles

    DisplayTitles(hStmt, cDisplaySize + 1, pFirstBinding);


    // Fetch and display the data

    bool fNoData = false;

    do {
        // Fetch a row

        if (iCount++ >= gHeight - 2)
        {
            int     nInputChar;
            bool    fEnterReceived = false;

            while (!fEnterReceived)
            {
                SetConsole(cDisplaySize + 2, TRUE);
                wprintf(L" F1 - Next page | F2 - First page | F3 - Quit to main menu | (%hd)", gHeight);
                SetConsole(cDisplaySize + 2, FALSE);

                nInputChar = _getch();
                wprintf(L"\n");
                if (nInputChar == 114)
                {
                    goto Exit;
                }
                else if (nInputChar == 112)
                {
                    fEnterReceived = true;
                }
                else if (nInputChar == 113) 
                {
                    DisplayResults(hStmt, cCols);
                }
                // else loop back to display prompt again
            }

            iCount = 1;
            DisplayTitles(hStmt, cDisplaySize + 1, pFirstBinding);
        }

        TRYODBC(hStmt, SQL_HANDLE_STMT, RetCode = SQLFetch(hStmt));

        if (RetCode == SQL_NO_DATA_FOUND)
        {
            fNoData = true;
        }
        else
        {

            // Display the data.   Ignore truncations

            for (pThisBinding = pFirstBinding;
                pThisBinding;
                pThisBinding = pThisBinding->sNext)
            {
                if (pThisBinding->indPtr != SQL_NULL_DATA)
                {
                    wprintf(pThisBinding->fChar ? DISPLAY_FORMAT_C : DISPLAY_FORMAT,
                        PIPE,
                        pThisBinding->cDisplaySize,
                        pThisBinding->cDisplaySize,
                        pThisBinding->wszBuffer);
                }
                else
                {
                    wprintf(DISPLAY_FORMAT_C,
                        PIPE,
                        pThisBinding->cDisplaySize,
                        pThisBinding->cDisplaySize,
                        L"<NULL>");
                }
            }
            wprintf(L" %c\n", PIPE);
        }
    } while (!fNoData);

    SetConsole(cDisplaySize + 2, TRUE);
    wprintf(L"%*.*s", cDisplaySize + 2, cDisplaySize + 2, L" ");
    SetConsole(cDisplaySize + 2, FALSE);
    wprintf(L"\n");

Exit:
    // Clean up the allocated buffers

    while (pFirstBinding)
    {
        pThisBinding = pFirstBinding->sNext;
        free(pFirstBinding->wszBuffer);
        free(pFirstBinding);
        pFirstBinding = pThisBinding;
    }
}

/************************************************************************
/* AllocateBindings:  Get column information and allocate bindings
/* for each column.
/*
/* Parameters:
/*      hStmt      Statement handle
/*      cCols       Number of columns in the result set
/*      *lppBinding Binding pointer (returned)
/*      lpDisplay   Display size of one line
/************************************************************************/

void AllocateBindings(HSTMT         hStmt,
    SQLSMALLINT   cCols,
    BINDING** ppBinding,
    SQLSMALLINT* pDisplay,
    bool isFile)
{
    SQLSMALLINT     iCol;
    BINDING* pThisBinding, * pLastBinding = NULL;
    SQLLEN          cchDisplay, ssType;
    SQLSMALLINT     cchColumnNameLength;

    *pDisplay = 0;

    for (iCol = 1; iCol <= cCols; iCol++)
    {
        pThisBinding = (BINDING*)(malloc(sizeof(BINDING)));
        if (!(pThisBinding))
        {
            fwprintf(stderr, L"Out of memory!\n");
            exit(-100);
        }

        if (iCol == 1)
        {
            *ppBinding = pThisBinding;
        }
        else
        {
            pLastBinding->sNext = pThisBinding;
        }
        pLastBinding = pThisBinding;


        // Figure out the display length of the column (we will
        // bind to char since we are only displaying data, in general
        // you should bind to the appropriate C type if you are going
        // to manipulate data since it is much faster...)

        TRYODBC(hStmt,
            SQL_HANDLE_STMT,
            SQLColAttribute(hStmt,
                iCol,
                SQL_DESC_DISPLAY_SIZE,
                NULL,
                0,
                NULL,
                &cchDisplay));


        // Figure out if this is a character or numeric column; this is
        // used to determine if we want to display the data left- or right-
        // aligned.

        // SQL_DESC_CONCISE_TYPE maps to the 1.x SQL_COLUMN_TYPE. 
        // This is what you must use if you want to work
        // against a 2.x driver.

        TRYODBC(hStmt,
            SQL_HANDLE_STMT,
            SQLColAttribute(hStmt,
                iCol,
                SQL_DESC_CONCISE_TYPE,
                NULL,
                0,
                NULL,
                &ssType));


        pThisBinding->fChar = (ssType == SQL_CHAR ||
            ssType == SQL_VARCHAR ||
            ssType == SQL_LONGVARCHAR);

        pThisBinding->sNext = NULL;

        // Arbitrary limit on display size
        if (isFile)cchDisplay = 50;
        else {
            if (cchDisplay > DISPLAY_MAX)
                cchDisplay = DISPLAY_MAX;
        }
        // Allocate a buffer big enough to hold the text representation
        // of the data.  Add one character for the null terminator

        pThisBinding->wszBuffer = (WCHAR*)malloc((cchDisplay + 1) * sizeof(WCHAR));

        if (!(pThisBinding->wszBuffer))
        {
            fwprintf(stderr, L"Out of memory!\n");
            exit(-100);
        }

        // Map this buffer to the driver's buffer.   At Fetch time,
        // the driver will fill in this data.  Note that the size is 
        // count of bytes (for Unicode).  All ODBC functions that take
        // SQLPOINTER use count of bytes; all functions that take only
        // strings use count of characters.

        TRYODBC(hStmt,
            SQL_HANDLE_STMT,
            SQLBindCol(hStmt,
                iCol,
                SQL_C_TCHAR,
                (SQLPOINTER)pThisBinding->wszBuffer,
                (cchDisplay + 1) * sizeof(WCHAR),
                &pThisBinding->indPtr));


        // Now set the display size that we will use to display
        // the data.   Figure out the length of the column name

        TRYODBC(hStmt,
            SQL_HANDLE_STMT,
            SQLColAttribute(hStmt,
                iCol,
                SQL_DESC_NAME,
                NULL,
                0,
                &cchColumnNameLength,
                NULL));

        pThisBinding->cDisplaySize = max((SQLSMALLINT)cchDisplay, cchColumnNameLength);
        if (pThisBinding->cDisplaySize < NULL_SIZE)
            pThisBinding->cDisplaySize = NULL_SIZE;

        *pDisplay += pThisBinding->cDisplaySize + DISPLAY_FORMAT_EXTRA;

    }

    return;

Exit:

    exit(-1);

    return;
}

/************************************************************************
/* DisplayTitles: print the titles of all the columns and set the
/*                shell window's width
/*
/* Parameters:
/*      hStmt          Statement handle
/*      cDisplaySize   Total display size
/*      pBinding        list of binding information
/************************************************************************/

void DisplayTitles(HSTMT     hStmt,
    DWORD     cDisplaySize,
    BINDING* pBinding)
{
    WCHAR           wszTitle[DISPLAY_MAX];
    SQLSMALLINT     iCol = 1;

    SetConsole(cDisplaySize + 2, TRUE);

    for (; pBinding; pBinding = pBinding->sNext)
    {
        TRYODBC(hStmt,
            SQL_HANDLE_STMT,
            SQLColAttribute(hStmt,
                iCol++,
                SQL_DESC_NAME,
                wszTitle,
                sizeof(wszTitle), // Note count of bytes!
                NULL,
                NULL));

        wprintf(DISPLAY_FORMAT_C,
            PIPE,
            pBinding->cDisplaySize,
            pBinding->cDisplaySize,
            wszTitle);
    }

Exit:

    wprintf(L" %c", PIPE);
    SetConsole(cDisplaySize + 2, FALSE);
    wprintf(L"\n");
    SetConsoleTextAttribute(hConsole, 15);

}


/************************************************************************
/* SetConsole: sets console display size and video mode
/*
/*  Parameters
/*      siDisplaySize   Console display size
/*      fInvert         Invert video?
/************************************************************************/

void SetConsole(DWORD dwDisplaySize,
    BOOL  fInvert)
{
    HANDLE                          hConsole;
    CONSOLE_SCREEN_BUFFER_INFO      csbInfo;

    // Reset the console screen buffer size if necessary

    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (fInvert) {
        SetConsoleTextAttribute(hConsole, BACKGROUND_BLUE | 15);
    }
    else {
        SetConsoleTextAttribute(hConsole, 15);
    }
}


/************************************************************************
/* HandleDiagnosticRecord : display error/warning information
/*
/* Parameters:
/*      hHandle     ODBC handle
/*      hType       Type of handle (HANDLE_STMT, HANDLE_ENV, HANDLE_DBC)
/*      RetCode     Return code of failing command
/************************************************************************/

void HandleDiagnosticRecord(SQLHANDLE      hHandle,
    SQLSMALLINT    hType,
    RETCODE        RetCode)
{
    SQLSMALLINT iRec = 0;
    SQLINTEGER  iError;
    WCHAR       wszMessage[1000];
    WCHAR       wszState[SQL_SQLSTATE_SIZE + 1];


    if (RetCode == SQL_INVALID_HANDLE)
    {
        fwprintf(stderr, L"Invalid handle!\n");
        return;
    }

    while (SQLGetDiagRec(hType,
        hHandle,
        ++iRec,
        wszState,
        &iError,
        wszMessage,
        (SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)),
        (SQLSMALLINT*)NULL) == SQL_SUCCESS)
    {
        // Hide data truncated..
        if (wcsncmp(wszState, L"01004", 5))
        {
            fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
        }
    }

}