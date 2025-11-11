import requests
import json
from datetime import datetime

# 🔑 Sua chave da API OpenWeather
API_KEY = "chave"
CIDADE = "Sao Jose dos Campos"
URL = f"https://api.openweathermap.org/data/2.5/forecast?q={CIDADE}&appid={API_KEY}&units=metric&lang=pt_br"

# 🌐 Buscar dados da API
res = requests.get(URL)
data = res.json()

# 📅 Data atual
hoje = datetime.now().strftime("%Y-%m-%d")

# 📦 Previsões para os próximos 3 dias (cada bloco = 3h)
blocos = data["list"][:24]  # 8 blocos por dia × 3 dias

# 🧠 Agrupar por dia
dias = {}
for bloco in blocos:
    dia = bloco["dt_txt"].split(" ")[0]
    if dia not in dias:
        dias[dia] = {"min": [], "max": []}
    dias[dia]["min"].append(bloco["main"]["temp_min"])
    dias[dia]["max"].append(bloco["main"]["temp_max"])

# 📊 Histórico
historico = {
    "datas": list(dias.keys())[:3],
    "temperaturas_min": [round(min(dias[d]["min"]), 1) for d in list(dias.keys())[:3]],
    "temperaturas_max": [round(max(dias[d]["max"]), 1) for d in list(dias.keys())[:3]]
}

# 🌤️ Dados atuais
atual = blocos[0]
dados = {
    "cidade": data["city"]["name"],
    "data": hoje,
    "temperatura_minima": round(atual["main"]["temp_min"], 1),
    "temperatura_maxima": round(atual["main"]["temp_max"], 1),
    "condicao": atual["weather"][0]["description"],
    "chuva_mm": round(atual.get("rain", {}).get("3h", 0), 1),
    "vento_kmh": round(atual["wind"]["speed"] * 3.6, 1),
    "umidade": atual["main"]["humidity"],
    "historico": historico,
    "recomendacoes": []
}

# 💡 Regras de recomendação
if dados["chuva_mm"] > 5:
    dados["recomendacoes"].append("Evitar aplicação de defensivos — chuva prevista.")
else:
    dados["recomendacoes"].append("Boa condição para aplicação de defensivos.")

if dados["umidade"] < 60:
    dados["recomendacoes"].append("Ideal para irrigação leve.")
else:
    dados["recomendacoes"].append("Monitorar umidade antes de irrigar.")

if 15 <= dados["temperatura_minima"] <= 30:
    dados["recomendacoes"].append("Temperatura ideal para germinação de sementes.")

# 💾 Salvar JSON
with open("dados_clima.json", "w", encoding="utf-8") as f:
    json.dump(dados, f, indent=4, ensure_ascii=False)

print("✅ Arquivo dados_clima.json atualizado com sucesso!")