package com.itu.greenhouse

import android.app.AlertDialog
import android.content.Intent
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.*
import androidx.appcompat.app.AppCompatActivity
import com.itu.greenhouse.ActividadesActivity

class ActividadesActivity : AppCompatActivity() {

    private lateinit var btnRegresar: ImageButton
    private lateinit var btnInicio: ImageButton
    private lateinit var btnMenu: ImageButton
    private lateinit var btnAgregar: Button
    private lateinit var lvActividades: ListView

    // Lista mutable de actividades
    private val actividades = mutableListOf(
        Actividad("Fumigar", false),
        Actividad("Regar", false),
        Actividad("Plantar algo", false),
        Actividad("Cerrar cortinas", false)
    )

    private lateinit var adapter: ActividadAdapter

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_actividades)

        btnRegresar = findViewById(R.id.btnRegresar)

        btnMenu  = findViewById(R.id.btnMenu)
        btnAgregar  = findViewById(R.id.btnAgregar)
        lvActividades = findViewById(R.id.lvActividades)

        adapter = ActividadAdapter()
        lvActividades.adapter = adapter

        btnRegresar.setOnClickListener { finish() }

        btnInicio.setOnClickListener {
            val intent = Intent(this, MainActivity::class.java)
            intent.flags = Intent.FLAG_ACTIVITY_CLEAR_TOP
            startActivity(intent)
        }

        btnMenu.setOnClickListener {
            startActivity(Intent(this, MenuActivity::class.java))
        }

        btnAgregar.setOnClickListener { mostrarDialogoAgregar() }

        // Bottom nav
        findViewById<LinearLayout>(R.id.navInicio).setOnClickListener {
            val intent = Intent(this, MainActivity::class.java)
            intent.flags = Intent.FLAG_ACTIVITY_CLEAR_TOP
            startActivity(intent)
        }
        findViewById<LinearLayout>(R.id.navAjustes).setOnClickListener {
            // TODO: navegar a AjustesActivity
        }
        findViewById<LinearLayout>(R.id.navIA).setOnClickListener {
            // TODO: navegar a IAActivity
        }
    }

    private fun mostrarDialogoAgregar() {
        val input = EditText(this)
        input.hint = "Nombre de la actividad"

        AlertDialog.Builder(this)
            .setTitle("Nueva actividad")
            .setView(input)
            .setPositiveButton("Agregar") { _, _ ->
                val nombre = input.text.toString().trim()
                if (nombre.isNotEmpty()) {
                    actividades.add(Actividad(nombre, false))
                    adapter.notifyDataSetChanged()
                } else {
                    Toast.makeText(this, "Escribe un nombre", Toast.LENGTH_SHORT).show()
                }
            }
            .setNegativeButton("Cancelar", null)
            .show()
    }

    // Modelo
    data class Actividad(val nombre: String, var completada: Boolean)

    // Adapter
    inner class ActividadAdapter : BaseAdapter() {

        override fun getCount() = actividades.size
        override fun getItem(pos: Int) = actividades[pos]
        override fun getItemId(pos: Int) = pos.toLong()

        override fun getView(pos: Int, convertView: View?, parent: ViewGroup): View {
            val view = convertView ?: LayoutInflater.from(this@ActividadesActivity)
                .inflate(R.layout.item_actividad, parent, false)

            val actividad = actividades[pos]
            val tvNombre   = view.findViewById<TextView>(R.id.tvNombreActividad)
            val cbCompletado = view.findViewById<CheckBox>(R.id.cbCompletado)

            tvNombre.text = actividad.nombre
            cbCompletado.isChecked = actividad.completada

            // Tachar texto si está completada
            tvNombre.paintFlags = if (actividad.completada)
                tvNombre.paintFlags or android.graphics.Paint.STRIKE_THRU_TEXT_FLAG
            else
                tvNombre.paintFlags and android.graphics.Paint.STRIKE_THRU_TEXT_FLAG.inv()

            cbCompletado.setOnCheckedChangeListener { _, isChecked ->
                actividades[pos].completada = isChecked
                notifyDataSetChanged()
            }

            return view
        }
    }
}