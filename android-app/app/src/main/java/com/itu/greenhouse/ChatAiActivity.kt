package com.itu.greenhouse

import android.os.Bundle
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.itu.greenhouse.databinding.ActivityChatAiBinding


class ChatAiActivity : AppCompatActivity() {

    private lateinit var binding: ActivityChatAiBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityChatAiBinding.inflate(layoutInflater)
        setContentView(binding.root)

        // Configurar botones de navegación
        binding.btnBack.setOnClickListener {
            onBackPressedDispatcher.onBackPressed()
        }

        binding.btnSend.setOnClickListener {
            val message = binding.etMessage.text.toString()
            if (message.isNotEmpty()) {
                // Aquí iría la lógica para enviar a la IA
                Toast.makeText(this, "Enviando: $message", Toast.LENGTH_SHORT).show()
                binding.etMessage.text.clear()
            }
        }

        // Puedes agregar listeners para los iconos inferiores aquí
    }
}