#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

//Nota: pentru fiecare comanda de login, logout data spre test, pentru a vedea actiunea procedurii este necesara ca baza de date sa 
//se inchida si deschida din nou pentru a vedea informatiile actualizate, acestea nu se modifica in timp real;

int callback(void *NotUsed, int argc, char **argv, 
                    char **azColName) {
    
    NotUsed = 0;
    
    for (int i = 0; i < argc; i++) {

        printf("%s ", argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    
    return 0;
}



int main(int argc, char** argv)
{
    sqlite3 *db; //database handle
    char *ErrMsg = 0;
    
    //open a new database connection -> its name are database name and the database handle
    int rc = sqlite3_open("test.db", &db);

    if ( rc != SQLITE_OK)
    {
        printf("Eroare la deschiderea bazei de date!\n");
        sqlite3_close(db);
        exit(1);
    }
    else
       printf("Database is opened!\n");
    
    //Introducem date in tabel

//    char *sql = "DROP TABLE IF EXISTS MESSENGER;"
//    "CREATE TABLE Messenger (ID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, username NVARCHAR NOT NULL, password NVARCHAR NOT NULL);";

   char *sql = 
   "CREATE TABLE Chat (id_Mesaj INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, id_sursa INT, id_destinatie INT, FOREIGN KEY (id_sursa) REFERENCES Messenger(ID), FOREIGN KEY (id_destinatie) REFERENCES Messenger(ID)); ";
    rc = sqlite3_exec(db, sql, callback, 0, &ErrMsg);
    if (rc != SQLITE_OK ) {
        
        fprintf(stderr, "SQL error: %s\n", ErrMsg);
        
        sqlite3_free(ErrMsg);        //If an error occurs then the last parameter points to the allocated error message.
        sqlite3_close(db);
        
        return 1;
    } 
    
    sqlite3_close(db);
    
    return 0;
}


