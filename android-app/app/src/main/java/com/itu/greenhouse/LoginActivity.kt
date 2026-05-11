package com.itu.greenhouse

import android.content.Intent
import android.os.Bundle
import android.widget.Button
import android.widget.EditText
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity

class LoginActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        // Si tu XML se llama login.xml, usa R.layout.login
        setContentView(R.layout.login)

        // 1. Referencias a los componentes
        val etUsuario = findViewById<EditText>(R.id.et_usuario)
        val etContrasena = findViewById<EditText>(R.id.et_contrasena)
        val btnContinuar = findViewById<Button>(R.id.btn_continuar)

        // Referencias para los textos clickables (Asegúrate de ponerles ID en el XML)
        val tvOlvidaste = findViewById<TextView>(R.id.tv_olvidaste)
        val tvRegistrate = findViewById<TextView>(R.id.tv_registrate)

        // 2. Lógica del botón Continuar
        btnContinuar.setOnClickListener {
            val usuario = etUsuario.text.toString().trim()
            val contrasena = etContrasena.text.toString().trim()

            if (usuario.isEmpty() || contrasena.isEmpty()) {
                Toast.makeText(this, "Por favor, llena todos los campos", Toast.LENGTH_SHORT).show()
            } else {
                // Aquí conectarías con tu lógica de BancoAri o Agro-tec
                Toast.makeText(this, "Validando usuario...", Toast.LENGTH_SHORT).show()
            }
        }

        // 3. Clic en "¿Olvidaste tu contraseña?"
        tvOlvidaste.setOnClickListener {
            // Ejemplo: Ir a una pantalla de recuperación (debes crearla)
            // val intent = Intent(this, RecuperarActivity::class.java)
            // startActivity(intent)
            Toast.makeText(this, "Función para recuperar clave", Toast.LENGTH_SHORT).show()
        }

        // 4. Clic en "Regístrate"
        tvRegistrate.setOnClickListener {
            // Ejemplo: Ir a la pantalla de registro (debes crearla)
            // val intent = Intent(this, RegistroActivity::class.java)
            // startActivity(intent)
            Toast.makeText(this, "Yendo a Registro", Toast.LENGTH_SHORT).show()
        }
    }
}