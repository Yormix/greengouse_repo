package com.itu.greenhouse

import android.os.Bundle
import android.widget.Button
import android.widget.ImageButton
import androidx.appcompat.app.AppCompatActivity
import com.itu.greenhouse.HistorialActivity

class HistorialActivity : AppCompatActivity() {

    private lateinit var btnRegresar: ImageButton
    private lateinit var btnNotificaciones: ImageButton
    private lateinit var btnSeleccionarOpcion: Button

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_historial)

        btnRegresar         = findViewById(R.id.btnRegresar)
        btnNotificaciones    = findViewById(R.id.btnNotificaciones)
        btnSeleccionarOpcion = findViewById(R.id.btnSeleccionarOpcion)

        btnRegresar.setOnClickListener { finish() }



        btnSeleccionarOpcion.setOnClickListener {
            // TODO: mostrar dropdown para filtrar por fecha/sensor
            mostrarSelector()
        }

        // TODO: cargar datos reales en las gráficas desde ViewModel
        // Si usas MPAndroidChart:
        // cargarGraficaTemperatura()
        // cargarGraficaHumedad()
        // cargarGraficaPH()
    }

    private fun mostrarSelector() {
        val opciones = arrayOf("Última semana", "Último mes", "Últimos 3 meses")
        val builder = android.app.AlertDialog.Builder(this)
        builder.setTitle("Selecciona un período")
        builder.setItems(opciones) { _, which ->
            btnSeleccionarOpcion.text = "${opciones[which]}  ▼"
            // TODO: recargar gráficas con el período seleccionado
        }
        builder.show()
    }
}