package com.itu.greenhouse.ui.theme
import android.content.Intent
import android.os.Bundle
import android.widget.LinearLayout
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.appcompat.widget.AppCompatButton
import com.itu.greenhouse.R

class SuccessActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        // Vinculamos el XML que perfeccionamos juntos
        setContentView(R.layout.cuenta_creada)



        // 1. Vincular las vistas por sus IDs
        val btnBack = findViewById<LinearLayout>(R.id.btn_back)
        val btnEmpezarAhora = findViewById<AppCompatButton>(R.id.btn_empezarahora)


        // 2. Acción para el botón "Regresar"
        btnBack.setOnClickListener {
            // Esto cierra la pantalla de éxito y vuelve al formulario de registro
            finish()
        }

        // 3. Acción para el botón principal "¡EMPEZAR AHORA!"
        btnEmpezarAhora.setOnClickListener {
            iniciarApp()
        }
    }

    private fun iniciarApp() {
        // Aquí debes poner la Activity que sigue después del registro
        // val intent = Intent(this, MainActivity::class.java)

        /* Tip de UX: Usamos estas banderas para que el usuario NO pueda
           regresar a esta pantalla de éxito una vez que ya entró a la app.
        */
        // intent.flags = Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TASK

        // startActivity(intent)

        Toast.makeText(this, "¡Registro completado!", Toast.LENGTH_SHORT).show()

        // Cerramos esta pantalla para que no quede en la memoria
        finish()
    }
}