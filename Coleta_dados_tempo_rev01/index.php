<?php
// Define localidade para datas em português
setlocale(LC_TIME, 'pt_BR.UTF-8');

// Tenta carregar o JSON
$json = @file_get_contents("dados_clima.json");
$data = json_decode($json, true);

// Verifica se o conteúdo foi carregado e é válido
if (!$data || !is_array($data)) {
    die("<p style='color:red'><strong>❌ Erro:</strong> Não foi possível carregar os dados climáticos.</p>");
}
?>
<!DOCTYPE html>
<html lang="pt-br">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Clima em <?= $data['cidade'] ?></title>
    <link rel="stylesheet" href="style.css">
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
        body {
            font-family: Arial, sans-serif;
            background: #f0f4f8;
            padding: 20px;
        }
        .card {
            background: white;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 0 10px rgba(0,0,0,0.1);
        }
        h2 {
            margin-top: 0;
        }
        ul {
            padding-left: 20px;
        }
        li {
            margin-bottom: 8px;
        }
        table {
            width: 100%;
            border-collapse: collapse;
        }
        th, td {
            padding: 8px;
            text-align: left;
        }
    </style>
</head>
<body>
    <div style="display: flex; gap: 20px; flex-wrap: wrap; justify-content: center;">
        <!-- Clima Atual -->
        <div class="card" style="flex: 1; min-width: 300px;">
            <h2>🌤️ Clima Atual</h2>
            <p><strong>Cidade:</strong> <?= $data['cidade'] ?></p>
            <?php
                $dataAtual = DateTime::createFromFormat('Y-m-d', $data['data']);
                $diaSemana = ucfirst(strftime('%A', $dataAtual->getTimestamp()));
                $dataFormatada = $dataAtual->format('d/m/Y');
            ?>
            <p><strong>Data:</strong> <?= $diaSemana ?>, <?= $dataFormatada ?></p>
            <p><strong>Temperatura Mínima:</strong> <?= $data['temperatura_minima'] ?> °C</p>
            <p><strong>Temperatura Máxima:</strong> <?= $data['temperatura_maxima'] ?> °C</p>
            <p><strong>Condição:</strong> <?= $data['condicao'] ?></p>
            <p><strong>Chuva prevista:</strong> <?= $data['chuva_mm'] ?> mm</p>
            <p><strong>Vento:</strong> <?= $data['vento_kmh'] ?> km/h</p>
            <p><strong>Umidade:</strong> <?= $data['umidade'] ?>%</p>
        </div>

        <!-- Recomendações Agrícolas -->
        <div class="card" style="flex: 1; min-width: 300px;">
            <h2>🌱 Recomendações Agrícolas</h2>
                <?php if (isset($data['recomendacoes']) && is_array($data['recomendacoes'])): ?>
                    <ul>
                        <?php foreach ($data['recomendacoes'] as $rec): ?>
                            <li><?= $rec ?></li>
                        <?php endforeach; ?>
                    </ul>
                <?php else: ?>
                    <p>Nenhuma recomendação disponível.</p>
                <?php endif; ?>
        </div>
        <!-- Adicionar campos de filtros -->
        <div style="margin-bottom: 20px;">
            <label>De: <input type="date" id="dataInicio"></label>
            <label>Até: <input type="date" id="dataFim"></label>
            <button onclick="filtrarHistorico()">Filtrar</button>
        </div>
        <!-- Histórico de Temperaturas -->
        <div class="card" style="flex: 1; min-width: 300px;">
            <h2>📈 Histórico de Temperaturas</h2>
            <table>
                <thead>
                    <tr>
                        <th>Data</th>
                        <th>Mínima (°C)</th>
                        <th>Máxima (°C)</th>
                    </tr>
                </thead>
                <tbody>
                    <?php
                    $datas = $data['historico']['datas'];
                    $mins = $data['historico']['temperaturas_min'];
                    $maxs = $data['historico']['temperaturas_max'];

                    for ($i = 0; $i < count($datas); $i++) {
                        $dataConvertida = DateTime::createFromFormat('Y-m-d', $datas[$i]);
                        $diaSemana = ucfirst(strftime('%A', $dataConvertida->getTimestamp()));
                        $dataFormatada = $dataConvertida->format('d/m/Y');
                        echo "<tr>
                                <td>{$diaSemana}, {$dataFormatada}</td>
                                <td>{$mins[$i]}</td>
                                <td>{$maxs[$i]}</td>
                              </tr>";
                    }
                    ?>
                </tbody>
            </table>
        </div>
    </div>

    <!-- Gráfico -->
    <canvas id="graficoTemperaturas" style="max-width: 700px; margin: 40px auto;"></canvas>
    <script>
        const labels = <?= json_encode($data['historico']['datas']) ?>;
        const minTemps = <?= json_encode($data['historico']['temperaturas_min']) ?>;
        const maxTemps = <?= json_encode($data['historico']['temperaturas_max']) ?>;

        const ctx = document.getElementById('graficoTemperaturas');

        new Chart(ctx, {
            type: 'line',
            data: {
                labels: labels,
                datasets: [
                    {
                        label: 'Temperatura Mínima (°C)',
                        data: minTemps,
                        borderColor: 'rgba(54, 162, 235, 1)',
                        backgroundColor: 'rgba(54, 162, 235, 0.2)',
                        fill: true,
                        tension: 0.3
                    },
                    {
                        label: 'Temperatura Máxima (°C)',
                        data: maxTemps,
                        borderColor: 'rgba(255, 99, 132, 1)',
                        backgroundColor: 'rgba(255, 99, 132, 0.2)',
                        fill: true,
                        tension: 0.3
                    }
                ]
            },
            options: {
                responsive: true,
                plugins: {
                    title: {
                        display: true,
                        text: 'Previsão de Temperaturas para Plantio'
                    }
                },
                scales: {
                    y: {
                        beginAtZero: false
                    }
                }
            }
        });
    </script>
    <script>
        const datas = <?= json_encode($data['historico']['datas']) ?>;
        const mins = <?= json_encode($data['historico']['temperaturas_min']) ?>;
        const maxs = <?= json_encode($data['historico']['temperaturas_max']) ?>;

        function filtrarHistorico() {
        const inicio = document.getElementById('dataInicio').value;
        const fim = document.getElementById('dataFim').value;

        if (!inicio || !fim) {
            alert("Selecione ambas as datas.");
            return;
        }

        const corpoTabela = document.querySelector("tbody");
        corpoTabela.innerHTML = "";

        for (let i = 0; i < datas.length; i++) {
            const dataAtual = datas[i];
            if (dataAtual >= inicio && dataAtual <= fim) {
            const dataObj = new Date(dataAtual);
            const diaSemana = dataObj.toLocaleDateString('pt-BR', { weekday: 'long' });
            const dataFormatada = dataObj.toLocaleDateString('pt-BR');

            const linha = `<tr>
                <td>${diaSemana.charAt(0).toUpperCase() + diaSemana.slice(1)}, ${dataFormatada}</td>
                <td>${mins[i]}</td>
                <td>${maxs[i]}</td>
            </tr>`;
            corpoTabela.innerHTML += linha;
            }
        }
        }
    </script>
</body>
</html>