import matplotlib.pyplot as plt

# Данные
threads = [1, 2, 4, 7, 8, 16, 20, 40]  # число потоков
times = [25.5924, 0.406391, 0.195698, 0.158127, 0.169453, 0.185482, 0.182828, 0.207136]  # время выполнения

# Построение графика
plt.plot(threads, times, 'bo-')  # b - синий, o - точки, - - линия
plt.xlabel('Число потоков')
plt.ylabel('Время выполнения (секунды)')
plt.title('Зависимость времени от числа потоков')
plt.grid(True)
plt.savefig('graph.png')