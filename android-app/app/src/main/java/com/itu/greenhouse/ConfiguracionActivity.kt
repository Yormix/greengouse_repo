package com.itu.greenhouse

import android.os.Bundle
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.itu.greenhouse.databinding.ActivityConfiguracionBinding

class ConfiguracionActivity : AppCompatActivity() {

    private lateinit var binding: ActivityConfiguracionBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityConfiguracionBinding.inflate(layoutInflater)
        setContentView(binding.root)

        // Botón de regreso
        binding.btnBack.setOnClickListener { finish() }

        // Opciones del menú (Cada ID debe existir en el XML)
        binding.rowAjustesGenerales.setOnClickListener {
            Toast.makeText(this, "Ajustes Generales", Toast.LENGTH_SHORT).show()
        }

        binding.rowAlertas.setOnClickListener {
            Toast.makeText(this, "Alertas y Notificaciones", Toast.LENGTH_SHORT).show()
        }

        binding.rowPersonalizacion.setOnClickListener {
            Toast.makeText(this, "Personalización", Toast.LENGTH_SHORT).show()
        }

        binding.rowPrivacidad.setOnClickListener {
            Toast.makeText(this, "Privacidad y Cuenta", Toast.LENGTH_SHORT).show()
        }

        binding.rowAcercaDe.setOnClickListener {
            Toast.makeText(this, "Acerca de / Soporte", Toast.LENGTH_SHORT).show()
        }

        // Navegación inferior
        binding.navInicio.setOnClickListener { finish() }

        binding.navAjustes.setOnClickListener {
            // Ya estamos en esta pantalla
        }

        binding.navIA.setOnClickListener {
            Toast.makeText(this, "Módulo IA próximamente", Toast.LENGTH_SHORT).show()
        }
    }
}