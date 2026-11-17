# algoritma_analizi_proje — Plagiarism Detector

CMP3005 Analysis of Algorithms · 2026 Güz · Bahçeşehir University · Dönem Projesi (%20)

`main_doc.txt` ve bir `documents/` klasörü verildiğinde, her belge için `main_doc`'taki
cümlelerin ne kadarının o belgeden (birebir ya da küçük düzenlemelerle) kopyalandığını
raporlayan ve örnek eşleşen cümle çiftlerini gösteren bir **console** programı. İki
sürümle gelir — bir **brute force** baseline ve **hashing** üzerine kurulu, belirgin
şekilde daha hızlı bir sürüm — ve ikisi aynı sonucu üretir.

Tüm yaklaşım, complexity ve gerekçeler için **[docs/design.md](docs/design.md)**;
ilerleme günlüğü için **[docs/progress.md](docs/progress.md)**; bekleyen kararlar için
**[docs/open-questions.md](docs/open-questions.md)**.

> **Dil:** Teknik **terimler English**, açıklamalar Türkçe. Kod, yorum ve identifier'lar
> English.

## Durum (Status)

| Bileşen | Durum |
|---------|-------|
| Materyal incelemesi & kapsam analizi | ✅ bitti |
| Tasarım belgesi | ✅ bitti |
| Repo yapısı & dokümantasyon | ✅ bitti |
| Brute force sürümü (`brute_force`) | ⏳ planlandı |
| Efficient sürümü (`better_solution`) | ⏳ planlandı |
| Örnek-set testi vs `expected_output.txt` | ⏳ planlandı |
| Benchmark'lar (timing table + plot) | ⏳ planlandı |
| Rapor (4–6 sayfa) | ⏳ planlandı |
| Sunum | ⏳ planlandı |

## Nasıl çalışır (özet)

- **Matching rule — word-set Jaccard.** Her cümle normalize edilmiş bir word token
  set'ine dönüşür; iki cümle `|A∩B| / |A∪B| ≥ THRESHOLD` olduğunda eşleşir. Bu,
  ödevdeki küçük düzenlemelere (kelime değiştir/ekle/sil, komşu kelime swap) dayanıklı
  ve sırasızdır.
- **Brute force** her `main_doc` cümlesini her document cümlesiyle karşılaştırır —
  Lecture 4 baseline'ı.
- **Efficient** document cümlelerini bir hash set'e (birebir kopyalar) ve bir
  `content word → cümleler` **inverted index**'ine önişler, sonra yalnızca ortak bir
  content word paylaşan candidate cümlelerle karşılaştırır — Lecture 7 space-for-time
  tradeoff'u. Aynı sonuç, çok daha az karşılaştırma.

## Klasör yapısı

```
algoritma_analizi_proje/
├── README.md              # bu dosya
├── .gitignore
├── main                   # (kod — placeholder) giriş + timing
├── brute_force            # (kod — placeholder) brute force detector
├── better_solution        # (kod — placeholder) efficient detector
├── docs/
│   ├── design.md          # tam tasarım (yaklaşım, complexity, gerekçeler)
│   ├── progress.md        # ilerleme günlüğü
│   ├── open-questions.md  # bekleyen kararlar
│   └── report/            # 4–6 sayfalık rapor + figürler
└── data/sample_set/       # verilen örnek (main_doc.txt, documents/, expected_output.txt)
```

Kod dosyaları (`main` / `brute_force` / `better_solution`) uygulama aşamasında `.cpp`
olur; ortak yardımcılar için bir `common.hpp` eklenir.

## Derleme ve çalıştırma (planlanan)

```
g++ -O2 -std=c++17 main.cpp brute_force.cpp better_solution.cpp -o plagiarism
./plagiarism              # bulunulan klasörde main_doc.txt + documents/ okur, raporu yazar (efficient)
./plagiarism --brute      # brute force sürümünü çalıştırır
./plagiarism --benchmark  # ikisini çalıştırıp timing yazdırır
```

## Test

`data/sample_set/` içinde çalıştır ve output'u `expected_output.txt` ile karşılaştır
(blok içindeki çift sırası önemli değil). İki sürüm de uyuşmalı.

## Ekip (Team)

- _İsim — öğrenci numarası — (rol)_
- _İsim — öğrenci numarası — (rol)_
- _İsim — öğrenci numarası — (rol)_

## AI kullanımı

Syllabus'a göre proje **Limited and Conditional Use** AI'ya izin veriyor. Her AI
yardımı, teslimle birlikte verilen **AI Use Declaration Form**'da beyan edilir ve **her
üye kodun her parçasını açıklayabilir**.

## Standartlar

- Repo dosyalarında **terimler English, açıklamalar Türkçe**; kod English.
- **Conventional Commits** (`feat:`, `fix:`, `docs:`, `test:`, `chore:`).
- `docs/progress.md` ve yukarıdaki status table her çalışma oturumunda güncel tutulur.
