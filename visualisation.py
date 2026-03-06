import matplotlib.pyplot as plt
import numpy as np

labels = ['Add', 'Remove', 'Consume', 'Mixed']
std_list  = [57.7, 20.3, 29.9, 34.3]
fast_list = [451.4, 238.7, 279.7, 41.8]

x = np.arange(len(labels))
width = 0.35

fig, ax = plt.subplots(figsize=(10, 5))

bars1 = ax.barh(x - width/2, std_list,  width, label='std::list',  color='#1f77b4')  # blue

bars2 = ax.barh(x + width/2, fast_list, width, label='fast_list',  color='#ff7f0e')  # orange

ax.set_ylabel('Operation')
ax.set_xlabel('Throughput (M ops/sec)')
ax.set_title('std::list vs fast_list Throughput')
ax.set_yticks(x)
ax.set_yticklabels(labels)
ax.legend()
ax.grid(axis='x', linestyle='--', alpha=0.4, color='#1a6fc4')

fig.patch.set_facecolor('#0d1117')
ax.set_facecolor('#0d1117')
ax.title.set_color('white')
ax.xaxis.label.set_color('white')
ax.yaxis.label.set_color('white')
ax.tick_params(colors='white')
for spine in ax.spines.values():
    spine.set_edgecolor('#1a6fc4')

for bar in bars1:
    ax.text(bar.get_width() + 0.3, bar.get_y() + bar.get_height()/2,
            f'{bar.get_width()}M', ha='left', va='center', fontsize=9, color='white')
for bar in bars2:
    ax.text(bar.get_width() + 0.3, bar.get_y() + bar.get_height()/2,
            f'{bar.get_width()}M', ha='left', va='center', fontsize=9, color='white')

plt.tight_layout()
plt.savefig('benchmark.png', dpi=150, facecolor=fig.get_facecolor())
plt.show()