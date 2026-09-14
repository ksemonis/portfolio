package com.example.project2360

import android.content.ContentValues
import android.content.Context
import android.database.Cursor
import android.database.sqlite.SQLiteDatabase
import android.database.sqlite.SQLiteOpenHelper

// Makes tables, and basic CRUD
class DatabaseHelper(context: Context) :
    SQLiteOpenHelper(context, DATABASE_NAME, null, DATABASE_VERSION) {

    companion object {
        private const val DATABASE_NAME = "app_database.db"
        private const val DATABASE_VERSION = 1

        // Table of users
        const val TABLE_USERS = "users"
        const val COLUMN_USERNAME = "username"
        const val COLUMN_PASSWORD = "password"

        // Table for inventory
        const val TABLE_ITEMS = "items"
        const val COLUMN_ID = "id"
        const val COLUMN_NAME = "name"
        const val COLUMN_QUANTITY = "quantity"
    }

    override fun onCreate(db: SQLiteDatabase) {
        val createUsersTable = """
            CREATE TABLE $TABLE_USERS (
                $COLUMN_USERNAME TEXT PRIMARY KEY,
                $COLUMN_PASSWORD TEXT
            )
        """.trimIndent()

        val createItemsTable = """
            CREATE TABLE $TABLE_ITEMS (
                $COLUMN_ID INTEGER PRIMARY KEY AUTOINCREMENT,
                $COLUMN_NAME TEXT,
                $COLUMN_QUANTITY INTEGER
            )
        """.trimIndent()

        db.execSQL(createUsersTable)
        db.execSQL(createItemsTable)
    }

    override fun onUpgrade(db: SQLiteDatabase, oldVersion: Int, newVersion: Int) {
        db.execSQL("DROP TABLE IF EXISTS $TABLE_USERS")
        db.execSQL("DROP TABLE IF EXISTS $TABLE_ITEMS")
        onCreate(db)
    }

    // User authentication
    fun checkUser(username: String, password: String): Boolean {
        val db = readableDatabase
        val cursor: Cursor = db.query(
            TABLE_USERS,
            arrayOf(COLUMN_USERNAME),
            "$COLUMN_USERNAME=? AND $COLUMN_PASSWORD=?",
            arrayOf(username, password),
            null, null, null
        )
        val exists = cursor.moveToFirst()
        cursor.close()
        return exists
    }

    // Adds new user
    fun addUser(username: String, password: String): Boolean {
        val db = writableDatabase
        val values = ContentValues().apply {
            put(COLUMN_USERNAME, username)
            put(COLUMN_PASSWORD, password)
        }
        val result = db.insert(TABLE_USERS, null, values)
        return result != -1L
    }

    // CRUD for items
    fun addItem(name: String, quantity: Int): Boolean {
        val db = writableDatabase
        val values = ContentValues().apply {
            put(COLUMN_NAME, name)
            put(COLUMN_QUANTITY, quantity)
        }
        val result = db.insert(TABLE_ITEMS, null, values)
        return result != -1L
    }

    fun getAllItems(): Cursor {
        val db = readableDatabase
        return db.query(TABLE_ITEMS, null, null, null, null, null, null)
    }

    fun updateItem(id: Int, name: String, quantity: Int): Boolean {
        val db = writableDatabase
        val values = ContentValues().apply {
            put(COLUMN_NAME, name)
            put(COLUMN_QUANTITY, quantity)
        }
        val result = db.update(TABLE_ITEMS, values, "$COLUMN_ID=?", arrayOf(id.toString()))
        return result > 0
    }

    fun deleteItem(id: Int): Boolean {
        val db = writableDatabase
        val result = db.delete(TABLE_ITEMS, "$COLUMN_ID=?", arrayOf(id.toString()))
        return result > 0
    }
}
