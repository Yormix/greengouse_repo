package com.itu.greenhouse

import android.os.Bundle
import android.widget.EditText
import android.widget.LinearLayout
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.appcompat.widget.AppCompatButton

class RegistroActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.registro)

        // Vincular vistas
        val btnBack = findViewById<LinearLayout>(R.id.btn_back)
        val etUsuario = findViewById<EditText>(R.id.et_usuario)
        val etContrasena = findViewById<EditText>(R.id.et_contrasena)
        val btnContinuar = findViewById<AppCompatButton>(R.id.btn_continuar)
        val tvIniciaSesion = findViewById<TextView>(R.id.tv_iniciosesion)

        // Acción para el botón de Regresar
        btnBack.setOnClickListener {
            onBackPressedDispatcher.onBackPressed()
            // O finish() si solo quieres cerrar esta pantalla
        }

        // Lógica del botón Continuar
        btnContinuar.setOnClickListener {
            val usuario = etUsuario.text.toString().trim()
            val password = etContrasena.text.toString().trim()

            if (usuario.isEmpty() || password.isEmpty()) {
                Toast.makeText(this, "Por favor, completa todos los campos", Toast.LENGTH_SHORT).show()
            } else {
                // Aquí iría tu lógica de registro (Firebase, API, etc.)
                Toast.makeText(this, "Registrando a: $usuario", Toast.LENGTH_SHORT).show()
            }
        }

        // Acción para "¿Ya tienes cuenta? Inicia sesión"
        tvIniciaSesion.setOnClickListener {
            // Intent para ir a la pantalla de Login
            // val intent = Intent(this, LoginActivity::class.java)
            // startActivity(intent)
            Toast.makeText(this, "Ir a Login", Toast.LENGTH_SHORT).show()
        }
    }
}