#include <iostream>
#include <iomanip>
#include <vector>

using namespace std;

class SimplexMethod {
	// строка таблицы
	struct Row {
		vector<double> a; // коэффициенты при x
		double b; // правая часть
	};

	int n; // число переменных
	int m; // число ограничений
	vector<Row> table; // симплекс таблица
	vector<double> c; // коэффициенты оптимизируемой функции
	vector<int> variables; // все переменные
	vector<int> basis; // базисные переменные
	vector<double> deltas; // дельты

	void CalculateDeltas(); // вычисление дельт
	int GetArgMaxDelta(); // вычисление номера минимальной дельты

public:
	SimplexMethod(int n, int m);

	void Read(); // ввод значений
	void Print(); // вывод таблицы

	void Solve(); // решение ОЗЛП
};

SimplexMethod::SimplexMethod(int n, int m) {
	this->n = n; // запоминаем количество переменных
	this->m = m; // запоминаем количество условий

	table = vector<Row>(m, { vector<double>(n), 0 }); // создаём таблицу
	c = vector<double>(n); // создаём вектор коэффициентов
}

// ввод значений
void SimplexMethod::Read() {
	cout << "Enter function coefficients (c): ";
	c = vector<double>(n); // создаём вектор коэффициентов

	// считываем коэффициенты оптимизируемой функции
	for (int i = 0; i < n; i++)
		cin >> c[i];

	cout << "Enter restrictions coefficients:" << endl;

	// считываем коэффициенты ограничений
	for (int i = 0; i < m; i++) {
		cout << "Enter restriction " << (i + 1) << ": ";

		for (int j = 0; j < n; j++)
			cin >> table[i].a[j];

		cin >> table[i].b;
	}

	variables.clear(); // очищаем переменные

	// добавляем переменные
	for (int i = 0; i < n; i++)
		variables.push_back(i);

	for (int i = 0; i < m; i++) {
		c.push_back(0); // добавляем нули в функцию
		variables.push_back(n + i); // добавляем доп переменные
		basis.push_back(n + i); // делаем их базисными

		// добавляем коэффициенты для переменных с коэффициентом 1, если они стоят на главной диагонали, иначе с нулём
		for (int j = 0; j < m; j++)
			table[i].a.push_back(i == j);
	}
}

// вывод таблицы
void SimplexMethod::Print() {
	int vars = variables.size();

	cout << endl;
	cout << "+-----+";

	for (int i = 0; i < vars; i++)
		cout << "-----------+";

	cout << endl;

	cout << "|  C  |";

	for (int i = 0; i < vars; i++)
		cout << " " << setw(9) << c[i] << " |";

	cout << endl;

	cout << "+-----+";

	for (int i = 0; i <= vars; i++)
		cout << "-----------+";

	cout << endl;

	cout << "|basis|";
	for (int i = 0; i < vars; i++)
		cout << "    x" << setw(2) << left << (i + 1) << "    |";

	cout << "     b     |" << endl;
	cout << "+-----+";

	for (int i = 0; i <= vars; i++)
		cout << "-----------+";

	cout << endl;

	for (int i = 0; i < m; i++) {
		cout << "| x" << setw(2) << left;

		if (i < basis.size())
			cout << (basis[i] + 1);
		else
			cout << "?";

		cout  << " |";

		for (int j = 0; j < table[i].a.size(); j++)
			cout << " " << setw(9) << table[i].a[j] << " |";

		cout << " " << setw(9) << table[i].b << " |" << endl;
	}

	cout << "+-----+";

	for (int i = 0; i <= vars; i++)
		cout << "-----------+";

	cout << endl;

	if (!deltas.size())
		return;

	cout << "|  D  |";

	for (int i = 0; i < deltas.size(); i++)
		cout << " " << setw(9) << deltas[i] << " |";

	cout << endl;

	cout << "+-----+";

	for (int i = 0; i <= vars; i++)
		cout << "-----------+";

	cout << endl;
}

// вычисление дельт
void SimplexMethod::CalculateDeltas() {
	deltas.clear(); // очищаем массив дельт

	// проходимся по всем переменным
	for (int i = 0; i <= variables.size(); i++) {
		double delta = 0;

		// вычилсяем дельту
		for (int j = 0; j < basis.size(); j++)
			delta += c[basis[j]] * (i < variables.size() ? table[j].a[i] : table[j].b);

		// вычитаем коэффициент функции
		if (i < variables.size())
			delta -= c[i];

		deltas.push_back(delta); // добавляем дельту в массив
	}
}

// вычисление номера минимальной дельты
int SimplexMethod::GetArgMaxDelta() {
	int imax = 0; // считаем, что первая дельта максимальна

	// проходимся по всем дельтам
	for (int i = 1; i < deltas.size(); i++)
		if (deltas[i] > deltas[imax]) // если дельта стала больше максимальной
			imax = i; // обновляем индекс максимума

	return imax; // возвращаем индекс максимума
}

// решение ОЗЛП
void SimplexMethod::Solve() {
	int iteration = 1; // начинаем с первой итерации

	while (true) {
		CalculateDeltas(); // рассчитываем дельты

		int jmax = GetArgMaxDelta(); // ищем индекс максимальной
		double maxDelta = deltas[jmax]; // получаем максимальную дельту

		cout << "Max delta: " << maxDelta << endl; // выводим максимальную дельту

		// если она не положительна
		if (maxDelta <= 0) {
			cout << "Plan is OK" << endl; // выводим, что план оптимален
			Print(); // выводим таблицу 
			break; // и выходим
		}

		cout << "Iteration " << iteration++ << ":" << endl; // выводим номер итерации
		cout << "Calculating deltas:" << endl;
		Print(); // выводим таблицу

		vector<double> Q(m); // создаём симплекс отношения
		int imin = -1;

		// идём по ограничениям
		for (int i = 0; i < m; i++) {
			if (table[i].a[jmax] == 0) { // если коэффициент равен 0
				Q[i] = 0; // то отношение равно нулю
			}
			else {
				Q[i] = table[i].b / table[i].a[jmax]; // вычисляем результат отношения

				// если оно отрицательно, то идём дальше
				if (Q[i] < 0)
					continue;

				// иначе обновляем минимальное симплекс отношение
				if (imin == -1 || Q[i] < Q[imin])
					imin = i;
			}
		}

		basis[imin] = jmax; // делаем переменную базисноц
		double pivot = table[imin].a[jmax]; // получаем опорный элемент
		
		cout << "Min Q: " << Q[imin] << endl; // выводим минимальное симплекс отношение
		cout << "x" << (jmax + 1) << " is new basis variable" << endl; // выводим новую базисную переменную
		cout << "Divide row " << (imin + 1) << " by " << pivot << endl; // делим строку на элемент

		// делим строку на элемент
		for (int i = 0; i < table[imin].a.size(); i++)
			table[imin].a[i] /= pivot;

		table[imin].b /= pivot;
		
		// вычитаем из всех остальных строк эту строку, умноженную на элемент в столбце jmax
		for (int i = 0; i < m; i++) {
			if (i == imin)
				continue;

			double value = table[i].a[jmax];

			for (int j = 0; j < table[i].a.size(); j++)
				table[i].a[j] -= table[imin].a[j] * value;

			table[i].b -= table[imin].b * value;
		}
	}

	cout << "Fmin: " << deltas[n + m] << endl; // выводим минимальное значение фукнции
}

int main() {
	int m;
	int n;

	cout << "Enter number of variables: ";
	cin >> n; // считываем число переменных
	cout << "Enter number of restrictions: ";
	cin >> m; // считываем число ограничений

	SimplexMethod method(n, m); // создаём метод

	method.Read(); // считыааем данные
	
	cout << "Initial simplex table: ";
	method.Print();  // выводим начальную таблицу

	method.Solve(); // решаем
}