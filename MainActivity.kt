package com.example.project2360

import android.database.Cursor
import android.os.Bundle
import android.widget.Button
import android.widget.EditText
import android.widget.GridView
import android.widget.SimpleCursorAdapter
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity

class MainActivity : AppCompatActivity() {
    private lateinit var db: DatabaseHelper
    private lateinit var gridView: GridView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        db = DatabaseHelper(this)
        gridView = findViewById(R.id.gridView)

        val nameField = findViewById<EditText>(R.id.editItemName)
        val quantityField = findViewById<EditText>(R.id.editQuantity)
        val addButton = findViewById<Button>(R.id.btnAdd)

        addButton.setOnClickListener {
            val name = nameField.text.toString()
            val qty = quantityField.text.toString().toIntOrNull()

            if (qty != null && db.addItem(name, qty)) {
                Toast.makeText(this, "Item added", Toast.LENGTH_SHORT).show()
                loadGrid()
            } else {
                Toast.makeText(this, "Failed to add item", Toast.LENGTH_SHORT).show()
            }
        }

        loadGrid()
    }

    private fun loadGrid() {
        val cursor: Cursor = db.getAllItems()
        val from = arrayOf(DatabaseHelper.COLUMN_NAME, DatabaseHelper.COLUMN_QUANTITY)
        val to = intArrayOf(android.R.id.text1, android.R.id.text2)

        val adapter = SimpleCursorAdapter(this, android.R.layout.simple_list_item_2, cursor, from, to, 0)
        gridView.adapter = adapter
    }
}
