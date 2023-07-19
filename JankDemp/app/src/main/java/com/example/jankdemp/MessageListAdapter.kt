package com.example.jankdemp

import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.recyclerview.widget.RecyclerView

class MessageListAdapter( private val messageList: Array<String>):
    RecyclerView.Adapter<MessageListAdapter.MessageHeaderViewHolder>() {
        class MessageHeaderViewHolder(view: View):RecyclerView.ViewHolder(view){
            fun bind(headerText :String){
                itemView.setOnClickListener{

                }
                itemView.findViewById<TextView>(R.id.messageHeader).text = headerText
            }
        }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): MessageHeaderViewHolder {
        val inflater = LayoutInflater.from(parent.context)
        return MessageHeaderViewHolder(
            inflater.inflate(R.layout.message_item, parent, false)
        )
    }

    override fun getItemCount(): Int {
        return messageList.size
    }

    override fun onBindViewHolder(holder: MessageHeaderViewHolder, position: Int) {
        holder.bind(messageList[position])
    }


}