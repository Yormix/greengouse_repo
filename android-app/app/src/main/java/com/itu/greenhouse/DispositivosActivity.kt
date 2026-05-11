package com.itu.greenhouse

import android.content.Intent
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.itu.greenhouse.databinding.ActivityDispositivosBinding

class DispositivosActivity : AppCompatActivity() {

    private lateinit var binding: ActivityDispositivosBinding

    private val devices = mutableListOf("Luces UV", "Jaime's Monster", "Puerta O1SDL")

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityDispositivosBinding.inflate(layoutInflater)
        setContentView(binding.root)

        loadDevices()

        binding.btnBack.setOnClickListener {
            finish()
        }

        binding.tvAddDevice.setOnClickListener {
            Toast.makeText(this, "Escaneando dispositivos Bluetooth...", Toast.LENGTH_SHORT).show()
        }

        // Al presionar MIS DISPOSITIVOS abre la segunda pantalla
        binding.btnMisDispositivos.setOnClickListener {
            startActivity(Intent(this, MisDispositivosActivity::class.java))
        }

        binding.navInicio.setOnClickListener {
            finish()
        }

        binding.navAjustes.setOnClickListener {
            startActivity(Intent(this, ConfiguracionActivity::class.java))
        }

        binding.navIA.setOnClickListener {
            startActivity(Intent(this, ChatAiActivity::class.java))
        }
    }

    private fun loadDevices() {
        binding.deviceControlList.removeAllViews()
        val inflater = LayoutInflater.from(this)

        for (deviceName in devices) {
            val itemView: View = inflater.inflate(R.layout.item_device, binding.deviceControlList, false)
            val tvName = itemView.findViewById<TextView>(R.id.tvDeviceName)
            tvName.text = deviceName
            itemView.setOnClickListener {
                Toast.makeText(this, "Seleccionado: $deviceName", Toast.LENGTH_SHORT).show()
            }
            binding.deviceControlList.addView(itemView)
        }
    }
}