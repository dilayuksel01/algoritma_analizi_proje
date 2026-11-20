# İlerleme Günlüğü (Progress Log)

Her işbirlikçinin projenin nerede olduğunu görebilmesi için çalışan bir kayıt. En yeni
giriş en üstte. Her giriş: ne yapıldı, sırada ne var, kararlar, açık sorular.

---

## 2026-11-20 — Ampirik Analiz (Benchmark) Tamamlandı

**Yapıldı**
- Yönerge gereği en az 5 farklı girdi boyutu için (4, 8, 12, 16, 20 doküman) otomatik hız ölçümü yapan `run_benchmark.py` yazıldı ve çalıştırıldı.
- Her boyut için 7 bağımsız çalıştırma yapılıp aykırı değerler elendi ve kırpılmış ortalama (trimmed mean) alındı.
- Sonuçlar:
  - 4 doküman: Brute 10.21 ms, Efficient 2.97 ms (3.44x hızlanma)
  - 8 doküman: Brute 20.84 ms, Efficient 5.74 ms (3.63x hızlanma)
  - 12 doküman: Brute 31.68 ms, Efficient 7.94 ms (3.99x hızlanma)
  - 16 doküman: Brute 40.59 ms, Efficient 9.89 ms (4.10x hızlanma)
  - 20 doküman: Brute 51.63 ms, Efficient 11.96 ms (4.32x hızlanma)
- Rapor için `docs/report/benchmark_table.md`, `benchmark_data.csv` ve yüksek çözünürlüklü çizgi grafikler (`docs/report/figures/benchmark_plot.png` ve `.pdf`) üretildi.

**Sırada**
- 4-6 sayfalık proje raporunun yazılması (Bölüm 2).
- Sunum hazırlığı (Bölüm 4).

---

## 2026-11-20 — Modüler C++ Kodlama Tamamlandı & Doğrulandı

**Yapıldı**
- Kök dizindeki uzantısız taslak dosyalar (`main`, `brute_force`, `better_solution`) kaldırılıp, temiz ve modüler `.cpp` / `.hpp` mimarisine geçildi:
  - `common.hpp`: Cümle bölme (abbreviation ve decimal korumalı), tokenization, Jaccard benzerliği ($O(|a|+|b|)$) ve elle yazılmış **64-bit Polynomial Rolling Hash** (`polynomialHash` ve `CustomStringHash`).
  - `brute_force.hpp` & `brute_force.cpp`: $O(S_m \cdot S_d \cdot L)$ karşılaştırma yapan baseline algoritma.
  - `better_solution.hpp` & `better_solution.cpp`: Kendi hash fonksiyonumuz ile oluşturulan Ters İndeks (Inverted Index), nadir kelimelere göre sıralama (rare words first), Pigeonhole prensipli Prefix Filtering ve Uzunluk Filtresi (Length Filter).
  - `main.cpp`: Standart çalıştırma, `--brute` ve `--benchmark` bayrakları. Çıktı formatı `expected_output.txt` ile birebir uyumlu.
- Program `g++ -O2 -std=c++17 main.cpp brute_force.cpp better_solution.cpp -o plagiarism` ile derlendi.
- `data/sample_set` üzerinde test edildi:
  - Beklenen tüm belgeler tam oranlarla tespit edildi (`document18.txt`: 26.7%, `document14.txt`: 16.7%, `document20.txt`: 13.3%, `document3.txt`: 6.7%, `document4.txt`: 3.3%, diğerleri: 0.0%).
  - Brute Force ve Efficient sürümlerinin **%100 aynı sonuçları** (`Identical matches: YES`) ürettiği doğrulandı.
  - 20 dokümanda dahi **4.3x hızlanma** ölçüldü.

**Sırada**
- Farklı doküman boyutlarında hız testi (Benchmark tablosu ve grafiği - Bölüm B).
- Rapor taslağının oluşturulması (Bölüm C).

## 2026-11-17 — Dokümanlar Türkçeleştirildi + GitHub'a push

**Yapıldı**
- Tüm dokümanlar **terimler English, gerisi Türkçe** olacak şekilde yeniden yazıldı.
- Repo yapısı ekibin flat düzenine (`main` / `brute_force` / `better_solution` kök
  dosyaları) hizalandı; gereksiz boş `src/` ve `scripts/` klasörleri kaldırıldı.
- Tasarım, plan, ilerleme günlüğü, açık sorular ve örnek set GitHub'a push edildi.

**Sırada**
- Ekip tasarımı onaylar (`docs/open-questions.md`).
- Kodlamaya geç: `common.hpp` → `brute_force` → `better_solution` → `main`.
- `THRESHOLD` + sentence splitter'ı `expected_output.txt`'e göre calibrate et.

---

## 2026-11-16 — Tasarım incelemesi & doğrulama

**Yapıldı**
- Tasarımın adversarial bir builder-check'i yapıldı, sonra iddiaları örnek sette
  **bağımsız olarak yeniden ölçüldü**.
- Matching rule'un sağlam olduğu doğrulandı: [0.262, 0.812] aralığındaki tek bir
  threshold 8/5/4/2/1'i üretiyor; hiçbir şey ders kapsamı dışına çıkmıyor.
- Gerçek bir defect yakalanıp düzeltildi: inverted index **content word**'lerle
  anahtarlanmalı, tüm token'larla değil — aksi halde efficient sürüm brute force'un
  ~0.95 katı iş yapıyor (hızlanma yok). Content-word index → ~0.12× (doğrulandı, hâlâ
  doğru).
- Sentence splitter sağlamlaştırıldı (abbreviation/decimal), threshold 0.5 sabitlendi,
  belge-başı bağımsız sayım, deterministik 5-çift seçimi ve exact-hash'in "fast path"
  (dur değil) olması eklendi. `docs/design.md` güncellendi.

**Açık kararlar (ekip)**
- Bu makineye bir C++ compiler kur vs ekip kendi makinelerinde derler.
- "Write hashing yourselves" kapsamını Teams rubric ile teyit et.
- Repo-docs dili (terimler English, gerisi Türkçe — varsayılan).

---

## 2026-11-16 — Proje kurulumu & tasarım

**Yapıldı**
- Tüm ders materyali (Lecture 1–7, çalışma rehberleri) ve proje örnek seti incelendi;
  tüm yaklaşımın Lecture 2, 3, 4, 7'ye sığdığı doğrulandı.
- Tasarım belgesi (`docs/design.md`) yazıldı.
- Repo yapısı, bu günlük ve README oluşturuldu.
- Verilen örnek set `data/sample_set/`'e kopyalandı.

**Kararlar**
- Dil: **C++** (C++17, yalnızca standard library).
- Matching rule: calibrate edilmiş threshold'lu **word-set Jaccard**.
- İki sürüm: brute force (Lecture 4) + **hashing** ile efficient (Lecture 7).
- Ödev izin verse de ders dışı tekniklerden bilinçli olarak kaçınılıyor.
