package com.itu.greenhouse

import android.content.Intent
import android.os.Bundle
import android.widget.Button
import androidx.appcompat.app.AppCompatActivity
import com.itu.greenhouse.MenuActivity

class MenuActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_menu)

        findViewById<Button>(R.id.btnActividadesPendientes).setOnClickListener {
            // TODO: navegar a ActividadesPendientesActivity
        }
stener {
            // TODO: navegar a ActividadesPendientesActivity
        }
        findViewById<Button>(R.id.btnAgenda).setOnClickListener {
            startActivity(Intent(this, AgendaActivity::class.java))
        }

        findViewById<Button>(R.id.btnSensores).setOnClickListener {
            // TODO: navegar a SensoresActivity
        }

        findViewById<Button>(R.id.btnHistorial).setOnClickListener {
            startActivity(Intent(this, HistorialActivity::class.java))
        }

        findViewById<Button>(R.id.btnDispositivos).setOnClickListener {
            // TODO: navegar a DispositivosActivity
        }
    }
}