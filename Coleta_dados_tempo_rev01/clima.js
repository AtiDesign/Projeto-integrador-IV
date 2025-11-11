import fs from 'fs';
const cidade = "Sao Jose dos Campos";
const apiKey = "chave";
const url = `https://api.openweathermap.org/data/2.5/forecast?q=${cidade}&appid=${apiKey}&units=metric&lang=pt_br`;

const response = await fetch(url);
const data = await response.json();

const hoje = data.list[0];
const historico = data.list.slice(0, 3);

const json = {
  cidade: data.city.name,
  data: hoje.dt_txt.split(" ")[0],
  temperatura_minima: hoje.main.temp_min,
  temperatura_maxima: hoje.main.temp_max,
  condicao: hoje.weather[0].description,
  chuva_mm: hoje.rain ? hoje.rain["3h"] || 0 : 0,
  vento_kmh: hoje.wind.speed * 3.6,
  umidade: hoje.main.humidity,
  historico: {
    datas: historico.map(h => h.dt_txt.split(" ")[0]),
    temperaturas_min: historico.map(h => h.main.temp_min),
    temperaturas_max: historico.map(h => h.main.temp_max)
  },
  recomendacoes: [
    hoje.rain && hoje.rain["3h"] > 5
      ? "Evitar aplicação de defensivos — chuva prevista."
      : "Boa condição para aplicação de defensivos.",
    hoje.main.humidity < 60
      ? "Ideal para irrigação leve."
      : "Monitorar umidade antes de irrigar.",
    "Temperatura ideal para germinação de sementes."
  ]
};

fs.writeFileSync('dados_clima.json', JSON.stringify(json, null, 2));
console.log("Arquivo dados_clima.json atualizado com sucesso!");