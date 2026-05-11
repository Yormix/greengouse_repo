package com.itu.greenhouse

import android.os.Bundle
import android.widget.Button
import android.widget.CalendarView
import android.widget.ImageButton
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.itu.greenhouse.R

class AgendaActivity : AppCompatActivity() {

    private lateinit var btnRegresar: ImageButton
    private lateinit var btnNotificaciones: ImageButton
    private lateinit var btnAgregarRecordatorio: Button
    private lateinit var btnGenerarReporte: Button
    private lateinit var calendarView: CalendarView
    private lateinit var tvUbicacion: TextView
    private lateinit var tvTemperaturaClima: TextView
    private lateinit var tvCondicion: TextView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_agenda)

        btnRegresar            = findViewById(R.id.btnRegresar)
        btnNotificaciones      = findViewById(R.id.btnNotificaciones)
        btnAgregarRecordatorio = findViewById(R.id.btnAgregarRecordatorio)
        btnGenerarReporte      = findViewById(R.id.btnGenerarReporte)
        calendarView           = findViewById(R.id.calendarView)
        tvUbicacion            = findViewById(R.id.tvUbicacion)
        tvTemperaturaClima     = findViewById(R.id.tvTemperaturaClima)
        tvCondicion            = findViewById(R.id.tvCondicion)

        btnRegresar.setOnClickListener { finish() }


        btnAgregarRecordatorio.setOnClickListener {
            // TODO: abrir diálogo para agregar recordatorio
            Toast.makeText(this, "Agregar recordatorio", Toast.LENGTH_SHORT).show()
        }

        btnGenerarReporte.setOnClickListener {
            // TODO: generar y exportar reporte
            Toast.makeText(this, "Generando reporte...", Toast.LENGTH_SHORT).show()
        }

        calendarView.setOnDateChangeListener { _, year, month, dayOfMonth ->
            // TODO: cargar mediciones del día seleccionado
            Toast.makeText(this, "Fecha: $dayOfMonth/${month + 1}/$year", Toast.LENGTH_SHORT).show()
        }

        // TODO: cargar clima real desde API (ej. OpenWeatherMap)
        cargarClima()
    }

    private fun cargarClima() {
        tvTemperaturaClima.text = "22"
        tvCondicion.text        = "Soleado"
        tvUbicacion.text        = "Hoy en Uruapan, Michoacán, México"
    }
}