"""
pyMDDatabase.py

Database services in sqlite3 for multi-dish runs.

This allows the server to efficiently store and retrieve data as updates progress.
"""

import os
import sys
import sqlite3

class MDDatabase():
    """
    MDDatabase

    Object to provide communication to the database, both storage and retrieval.

    Basic premise: All accesses should open the database, retrieve data or change it,
    then close it.
    """

    def dbOpen(self):
        """
        dbOpen

        Method to open the DB.
        """
        if (None == self.conn):
            self.conn = sqlite3.connect(self.db)
        if (None == self.cursor):
            self.cursor = self.conn.cursor()

    def dbClose(self):
        """
        dbClose

        Method to close the DB.
        """
        if (None != self.conn):
            self.conn.commit()   # Save the changes!
            if (None != self.cursor):
                self.cursor.close()     
                self.cursor = None
            self.conn = None
    
    def dbDoSQL(self,sqlstr):
        """
        dbDoSQL

        Method to perform an SQL action and then return either the status value (for INSERT or UPDATE)
        or the dataset (for SELECT).
        """
        if (None != self.cursor):
            self.cursor.execute(sqlstr)
            self.conn.commit()

    def handleSQL(self,sqlstr):
        """
        handleSQL

        Convenience routine to open, perform SQL, and close the database.
        """
        self.dbOpen()
        dbResult = self.dbDoSQL(sqlstr)
        self.dbClose()
        return(dbResult)    

    def __init__(self,dbfile):
        """
        __init__

        sqlite3 stores database as a single file, so dbfile tells the object
        which database it will be using. Do whatever is needed to start things off.
        """
        # Other considerations... existence-checking? Base path setting?
        self.db = dbfile
        self.conn = None
        self.cursor = None

    # def destruct

    def createTables(self):
        """
        createTables

        Method to create the tables needed for our data during a run.
        """
        if ("" != self.db):
            print "Create Table md SQL..."
            mdTableSQL = "CREATE TABLE IF NOT EXISTS md " +  \
                "(md_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, " +  \
                "md_name TEXT, " + \
                "md_update INT, " + \
                "popsize INT, " + \
                "avg_fit REAL, " + \
                "avg_metrate REAL, " + \
                "avg_gest REAL, " + \
                "avg_age REAL, " + \
                "op_not INT, " + \
                "op_nan INT, " + \
                "op_and INT, " + \
                "op_orn INT, " + \
                "op_oro INT, " + \
                "op_ant INT, " + \
                "op_nor INT, " + \
                "op_xor INT, " + \
                "op_eqn INT, " + \
                "pop_div_base_gen REAL, " + \
                "pop_div_base_phen REAL, " + \
                "pop_div_tide_gen REAL, " + \
                "pop_div_tide_phen REAL " + \
                ");"
            dbres = self.handleSQL(mdTableSQL)
            print " done, status = %s." % (str(dbres))

            print "Create Table sd..." 
            sdTableSQL = "CREATE TABLE IF NOT EXISTS sd " + \
                "(sd_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, " + \
                "sd_name TEXT, " + \
                "sd_update INT, " + \
                "sd_popsize INT, " + \
                "sd_natives INT, " + \
                "sd_immigrants INT, " + \
                "sd_div_base_gen REAL, " + \
                "sd_div_base_phen REAL, " + \
                "sd_div_tide_gen REAL, " + \
                "sd_div_tide_phen REAL " + \
                ");"
            dbres = self.handleSQL(sdTableSQL)
            print " done, status = %s." % (str(dbres))
            
            print "Create Table org..." 
            orgTableSQL = "CREATE TABLE IF NOT EXISTS org " + \
                "(org_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, " + \
                "md_cell_id INT, " + \
                "org_name TEXT, " + \
                "org_ancestor_id INT, " + \
                "org_born INT, " + \
                "org_killed INT " + \
                ");"
            dbres = self.handleSQL(orgTableSQL)
            print " done, status = %s." % (str(dbres))

            print "Create Table ou..." 
            ouTableSQL = "CREATE TABLE IF NOT EXISTS ou " + \
                "(ou_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, " + \
                "ou_org_id INT, " + \
                "ou_update INT, " + \
                "ou_fit REAL, " + \
                "ou_metrate REAL, " + \
                "ou_gest REAL, " + \
                "ou_age INT " + \
                ");"
            dbres = self.handleSQL(ouTableSQL)
            print " done, status = %s." % (str(dbres))

    def mkQs(self,n):
        qs = "?"
        if (1 < n):
            for ii in range(n-1):
                qs += ", ?"
        return(qs)
            
    def addMDUpdate(self,name,update,popsize,avgfit,avgmetrate,avggest,avgage):
        """
        addMDUpdate

        Method to store the information about the whole multidish for an update.
        """
        # Load input tuple
        intuple = (name,update,popsize,avgfit,avgmetrate,avggest,avgage,)
        mdUpdateSQL = "INSERT INTO md VALUES (" + self.mkQs(len(intuple)) + ");" 
        self.dbOpen()
        if (None != self.cursor):
            self.cursor.execute(mdUpdateSQL,intuple)
        self.dbClose()

    def addRowToTable(self,table,intuple):
        """
        addRowToTable

        Method to generically add a row of values to a table. 

        Inputs:
        table = name of the table to add to
        intuple = a tuple containing the values to add

        No checking is done to match the number of values to the number of columns in the table.
        """

        rowSQL = "INSERT INTO " + table + " VALUES (" + self.mkQs(len(intuple)) + ");" 
        print rowSQL
        self.dbOpen()
        if (None != self.cursor):
            self.cursor.execute(rowSQL,intuple)
        self.dbClose()
        

    def getRowsFromTable(self,table,where,intuple):
        """
        getRowsFromTable

        Method to retrieve rows according to selection criteria.

        Parameters in "where" clause to be supplied via the "intuple" input.
        """
        if ("" != self.db):
            selectSQL = "SELECT * FROM " + table + " " + where
            print selectSQL
            self.dbOpen()
            if (None != self.cursor):
                print "execute SELECT"
                self.dbresult = self.cursor.execute(selectSQL,intuple)
                return(self.dbresult)
            self.dbClose()

    def getAllFromTable(self,table):
        """
        getAllFromTable

        Method to retrieve the complete contents of a table.
        """
        if ("" != self.db):
            # self.dbresult = self.getRowsFromTable(table,"WHERE 1=1",())
            self.dbresult = self.getRowsFromTable(table,"",())
            print type(self.cursor)
            for row in self.dbresult:
                print row
            return(self.dbresult)
        else:
            self.dbresult = -1
            return(self.bdresult)

