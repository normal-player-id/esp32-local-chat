
const form = document.querySelector("#chat-form");
const nameInput = document.querySelector("#name");
const messageInput = document.querySelector("#message");
const messagesBox = document.querySelector("#messages");
const statusText = document.querySelector("#status");

let lastId = 0;
let busy = false;

nameInput.value = localStorage.getItem("chatName") || "";

function addMessage(item)
{
  const bubble = document.createElement("div");
  bubble.className = "bubble";

  const name = document.createElement("strong");
  name.textContent = item.name;

  const text = document.createElement("div");
  text.textContent = item.message;

  bubble.append(name, text);
  messagesBox.appendChild(bubble);
  messagesBox.scrollTop = messagesBox.scrollHeight;
}

async function refreshMessages() 
{
  if (busy) return;

  busy = true;

  try 
  {
    const response = await fetch
    (
      `/api/messages?after=${lastId}`,
      { cache: "no-store" }
    );

    if (!response.ok) throw new Error("Gagal mengambil pesan");

    const items = await response.json();

    for (const item of items) {
      addMessage(item);
      lastId = Math.max(lastId, item.id);
    }

    statusText.textContent = "Terhubung ke server lokal";
  } catch 
  {
    statusText.textContent = "Koneksi terputus, mencoba lagi...";
  } finally 
  {
    busy = false;
  }
}

form.addEventListener("submit", async (event) => {
  event.preventDefault();

  const name = nameInput.value.trim();
  const message = messageInput.value.trim();

  if (!name || !message) return;

  nameInput.value = name;
  localStorage.setItem("chatName", name);

  const body = new URLSearchParams({ name, message });

  try {
    const response = await fetch("/api/send", {
      method: "POST",
      headers: {
        "Content-Type": "application/x-www-form-urlencoded"
      },
      body
    });

    if (!response.ok) throw new Error("Pesan gagal dikirim");

    messageInput.value = "";
    await refreshMessages();
  } catch {
    statusText.textContent = "Gagal mengirim pesan. Coba lagi.";
  }
});

refreshMessages();
setInterval(refreshMessages, 1000);
