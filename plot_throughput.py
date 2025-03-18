import pandas as pd
import matplotlib.pyplot as plt

# Carregar os CSVs
with_congestion = pd.read_csv('throughput_with_congestion.csv')
without_congestion = pd.read_csv('throughput_without_congestion.csv')
with_loss = pd.read_csv('throughput_with_loss.csv')

# Criar figura com 3 subplots
plt.figure(figsize=(12, 8))

# Gráfico 1: Com congestionamento
plt.subplot(3, 1, 1)
plt.plot(with_congestion['Tempo'], with_congestion['Vazao'], label='Com Congestionamento')
plt.yscale('log')  # Escala logarítmica no eixo y
plt.xlabel('Tempo')
plt.ylabel('Vazão (pacotes/s)')
plt.title('Vazão com Controle de Congestionamento')
plt.legend()

# Gráfico 2: Sem congestionamento
plt.subplot(3, 1, 2)
plt.plot(without_congestion['Tempo'], without_congestion['Vazao'], label='Sem Congestionamento', color='orange')
plt.xlabel('Tempo')
plt.ylabel('Vazão (pacotes/s)')
plt.title('Vazão sem Controle de Congestionamento')
plt.legend()

# Gráfico 3: Com perdas (servidor)
plt.subplot(3, 1, 3)
plt.plot(with_loss['Tempo'], with_loss['Vazao'], label='Com Perdas', color='green')
plt.xlabel('Tempo')
plt.ylabel('Vazão (pacotes/s)')
plt.title('Vazão no Servidor com Perdas')
plt.legend()

# Ajustar layout e salvar
plt.tight_layout()
plt.savefig('throughput_plots.png')
plt.show()