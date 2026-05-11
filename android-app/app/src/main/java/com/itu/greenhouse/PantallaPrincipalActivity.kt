package com.itu.greenhouse

import android.content.Intent
import android.os.Bundle
import android.widget.LinearLayout
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import com.itu.greenhouse.R
import com.itu.greenhouse.LoginActivity

class PantallaPrincipalActivity : AppCompatActivity() {

    private lateinit var btnSalir: android.widget.ImageButton
    private lateinit var tvBienvenido: TextView
    private lateinit var tvHumedad: TextView
    private lateinit var tvTemperatura: TextView
    private lateinit var tvPH: TextView
    private lateinit var navInicio: LinearLayout
    private lateinit var navAjustes: LinearLayout
    private lateinit var navIA: LinearLayout
    private lateinit var btnMenu: android.widget.ImageButton

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_pantalla_principal)

        btnSalir       = findViewById(R.id.btnSalir)
        tvBienvenido   = findViewById(R.id.tvBienvenido)
        tvHumedad      = findViewById(R.id.tvHumedad)
        tvTemperatura  = findViewById(R.id.tvTemperatura)
        tvPH           = findViewById(R.id.tvPH)
        navInicio      = findViewById(R.id.navInicio)
        navAjustes     = findViewById(R.id.navAjustes)
        navIA          = findViewById(R.id.navIA)
        btnMenu        = findViewById(R.id.btnMenu)

        // Salir -> regresa al login
        btnSalir.setOnClickListener {
            val intent = Intent(this, LoginActivity::class.java)
            intent.flags = Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TASK
            startActivity(intent)
        }

        // Abrir menú
        btnMenu.setOnClickListener {
            startActivity(Intent(this, MenuActivity::class.java))
        }

        // Bottom nav
        navInicio.setOnClickListener { /* ya estamos aquí */ }
        navAjustes.setOnClickListener {
            // TODO: navegar a AjustesActivity
        }
        navIA.setOnClickListener {
            // TODO: navegar a IAActivity
        }

        // TODO: cargar datos reales desde ViewModel/Repository
        cargarDatos()
    }

    private fun cargarDatos() {
        // Aquí conectas con tu ViewModel para traer los valores del sensor
        tvHumedad.text     = "37.7 %"
        tvTemperatura.text = "0.00°C"
        tvPH.text          = "60%"
    }
}