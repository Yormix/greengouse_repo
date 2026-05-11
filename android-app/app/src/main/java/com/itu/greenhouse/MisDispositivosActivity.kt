package com.itu.greenhouse

import android.content.Intent
import android.os.Bundle
import android.view.LayoutInflater
import android.widget.Button
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import android.widget.Toast
import com.itu.greenhouse.databinding.ActivityMisDispositivosBinding

data class DeviceControl(
    val id: String,
    val name: String,
    val type: DeviceType,
    var estado: String = "Desconocido"
)

enum class DeviceType { DOOR, SWITCH }

class MisDispositivosActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMisDispositivosBinding

    private val devices = listOf(
        DeviceControl("1", "Luces UV", DeviceType.SWITCH, "Apagado"),
        DeviceControl("2", "Jaime's Monster", DeviceType.SWITCH, "Apagado"),
        DeviceControl("3", "Puerta O1SDL", DeviceType.DOOR, "Cerrada")
    )

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMisDispositivosBinding.inflate(layoutInflater)
        setContentView(binding.root)

        binding.btnBack.setOnClickListener { finish() }

        loadDeviceControls()

        binding.navInicio.setOnClickListener { finish() }
        binding.navAjustes.setOnClickListener {
            startActivity(Intent(this, ConfiguracionActivity::class.java))
        }
        binding.navIA.setOnClickListener {
            startActivity(Intent(this, ChatAiActivity::class.java))
        }
    }

    private fun loadDeviceControls() {
        binding.deviceControlList.removeAllViews()
        val inflater = LayoutInflater.from(this)

        for (device in devices) {
            val itemView = inflater.inflate(R.layout.item_device_control, binding.deviceControlList, false)

            val tvLabel      = itemView.findViewById<TextView>(R.id.tvDeviceLabel)
            val btnPrimary   = itemView.findViewById<Button>(R.id.btnPrimary)
            val btnSecondary = itemView.findViewById<Button>(R.id.btnSecondary)
            val tvEstado     = itemView.findViewById<TextView>(R.id.tvEstado)

            tvLabel.text  = device.name
            tvEstado.text = "Estado: ${device.estado}"

            when (device.type) {
                DeviceType.DOOR -> {
                    btnPrimary.text = "Abrir"
                    btnSecondary.text = "Cerrar"
                    btnPrimary.setOnClickListener {
                        device.estado = "Abierta"
                        tvEstado.text = "Estado: Abierta"
                    }
                    btnSecondary.setOnClickListener {
                        device.estado = "Cerrada"
                        tvEstado.text = "Estado: Cerrada"
                    }
                }
                DeviceType.SWITCH -> {
                    btnPrimary.text = "Encender"
                    btnSecondary.text = "Apagar"
                    btnPrimary.setOnClickListener {
                        device.estado = "Encendido"
                        tvEstado.text = "Estado: Encendido"
                    }
                    btnSecondary.setOnClickListener {
                        device.estado = "Apagado"
                        tvEstado.text = "Estado: Apagado"
                    }
                }
            }
            binding.deviceControlList.addView(itemView)
        }
    }
}