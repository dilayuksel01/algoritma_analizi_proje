# Tasarım Belgesi — Plagiarism Detector

**Ders:** CMP3005 Analysis of Algorithms · 2026 Güz · Bahçeşehir University
**Ağırlık:** Ders notunun %20'si
**Dil:** C++ (C++17), yalnızca standard library
**Durum:** TASARIM — ekip onayı bekliyor
**Son güncelleme:** 2026-11-17

> **Dil kuralı:** Teknik **terimler English** bırakılır (brute force, hashing,
> Jaccard, complexity, inverted index …); açıklamalar Türkçe yazılır. Kod,
> yorumlar, identifier'lar ve pseudocode English'tir. Bu belge yaklaşımın tek
> kaynağıdır; adım adım uygulama planı buradan türetilir.

> **Örnek set üzerinde doğrulandı (2026-11-16, bağımsız olarak yeniden ölçüldü).**
> `main_doc.txt` tam **30** cümleye bölünüyor; corpus'ta **2095** document cümlesi
> var. Matching rule, **[0.262, 0.812]** aralığındaki her threshold için belge
> başına beklenen sayıları (8/5/4/2/1) birebir üretiyor (biz **0.5** kullanıyoruz).
> Index'i **content word**'lerle kurmak, candidate karşılaştırmalarını brute
> force'un **~0.12 katına** düşürürken sonucu bozmuyor. Abbreviation/decimal
> kaynaklı sentence splitting riski gerçek (örnek sette: `St.`×7, `Mr.`×4,
> decimal ×67).

---

## 1. Problem ve amaç

`main_doc.txt` ve bir `documents/` klasörü (`document1.txt … documentN.txt`)
verildiğinde, her belge için `main_doc`'taki cümlelerin ne kadarının o belgeden
(birebir ya da küçük düzenlemelerle) kopyalandığını raporla ve örnek eşleşen cümle
çiftlerini göster.

İki ayrı sürüm zorunlu ve ikisi **aynı sonucu** üretmeli:

1. **Brute force** — doğrudan, sezgisel baseline (Lecture 4).
2. **Efficient** — belirgin şekilde daha hızlı; **hashing** kullanır (Lecture 7,
   space-for-time tradeoff / prestructuring).

**Neden:** notla değerlendirilen bir ders projesi. Öncelikler (sırayla): gizli test
setinde (~200 belge) correctness, brute force'tan belirgin şekilde hızlı bir
efficient sürüm, titiz bir rapor (Big-O + empirical timing), temiz kod, ve **her
üyenin kodun herhangi bir parçasını açıklayabildiği** bir sunum.

---

## 2. Goals / Non-goals / Anti-goals

**Goals**
- Örnek sette `expected_output.txt`'i birebir üret.
- Efficient sürüm brute force'tan ölçülebilir şekilde hızlı, aynı sonuçla.
- Raporda her adımın doğru Big-O'su + empirical timing (table + plot, ≥5 boyut).
- Her ekip üyesinin her satırı açıklayabileceği kadar sade kod.
- Ders içeriğinde kal (Lecture 2, 3, 4, 7).

**Non-goals**
- Gelişmiş NLP yok (stemming, synonyms, semantic similarity).
- External library yok — yalnızca standard C++ library.
- GUI yok — console programı.

**Anti-goals (teknik olarak çalışsa bile başarısızlık sayılır)**
- Ekipten kimsenin Q&A'da açıklayamadığı bir çözüm.
- Savunmayı zorlaştıran, ders dışı teknikler — edit distance / dynamic
  programming, suffix tree, KMP içi mekaniği, TF-IDF / cosine similarity.
  *(Ödev, efficient sürüm için ders dışı tekniklere açıkça izin veriyor — "Any
  text searching algorithm and/or data structure can be used even if not discussed
  in class" — ama biz bilinçli olarak kullanmıyoruz.)*
- Core matching için built-in text search (`find` / `indexOf` / `regex`) kullanmak
  (ödev yasaklıyor).

---

## 3. Kısıtlar (ödev + syllabus)

- C++ ya da Java → **C++**. Core matching ve hashing'i kendimiz yazıyoruz. Core için
  `find`/`indexOf`/`regex` yok. Standard container'lar serbest.
- Program `main_doc.txt` ve `documents/`'ı kendisi okur; user input yok.
- Output: eşleşen her belge için bir blok, en benzerden en aza sıralı; belge başına
  en benzer 5 cümle çiftine kadar; format `expected_output.txt`'e uygun.
- **Similarity rate** = (bir belgedeki ≥1 cümleyle eşleşen `main_doc` cümlesi sayısı)
  ÷ (toplam `main_doc` cümlesi) × 100.
- Teslim: tek bir `.pdf` (kaynak kod seçilebilir metin olarak + rapor), üyelerin
  öğrenci numaralarıyla adlandırılır, Teams'teki Turnitin sekmesine yüklenir.
- **AI kullanımı = "Limited and Conditional Use"** (syllabus). AI kullanıldıysa bir
  **AI Use Declaration Form** zorunlu. Her üye kodun her parçasını açıklayabilmeli.

---

## 4. Yaklaşım ve değerlendirilen alternatifler

### 4.1 Matching rule — word-set Jaccard

Her cümleyi normalize edip bir **word token set**'ine çevir. A ve B set'leri için:

```
Jaccard(A, B) = |A ∩ B| / |A ∪ B|        eşleşme  ⇔  Jaccard ≥ THRESHOLD
```

- Örnekteki her düzenleme tipine dayanıklı: kelime değiştir / ekle / sil veya iki
  komşu kelimeyi yer değiştir. Set sırasız olduğu için swap bedava; ~20 kelimelik
  bir cümlede tek kelimelik düzenleme Jaccard'ı yüksek tutar (~0.9).
- `THRESHOLD`, `expected_output.txt`'e karşı **örnek sette calibrate** edilir.
  Doğrulandı (2026-11-16): **[0.262, 0.812]** aralığındaki her threshold belge başına
  sayıları (8/5/4/2/1) birebir üretir. Biz **0.5** kullanıyoruz — aralığın ortası,
  gizli setteki daha kısa (ve daha düşük skorlayan) düzenlenmiş cümlelere karşı
  dayanıklı. Guard: `Jaccard(∅, ∅) = 0`.
- *Length-aware seçenek (opsiyonel):* çok kısa cümleler için küçük bir **mutlak**
  token farkını da kabul et; böylece 6 kelimelik cümledeki iki kelimelik düzenleme
  kaçmaz. Varsayılan kapalı; sadece gizli set gerektirirse aç.
- **Sayım belge bazında ve bağımsızdır:** iki belgedeki cümlelerle eşleşen bir
  `main_doc` cümlesi **ikisine de** sayılır (rate belge başına tanımlı); her cümleyi
  tek bir "best" belgeye atamayız.
- *Not — Jaccard neden savunulabilir?* Ödev matching rule'u takımın tanımlamasına
  açıkça izin veriyor; Jaccard ise temel set intersection/union'dır, ders dışı bir
  makine değil. Herkes açıklayabildiği sürece Q&A'da güvenlidir.

### 4.2 Brute force (baseline, Lecture 4)

Her `main_doc` cümlesini **her** document cümlesiyle karşılaştır ve Jaccard hesapla.
Bu, "her adayı dene" brute-force stratejisidir ve dersin çerçevesi birebir uyar:
*önce baseline, sonra tekrarlanan işi ele al.*

### 4.3 Efficient (hashing, Lecture 7)

Tüm document cümlelerini bir kez önişleyip iki yapı kur (space-for-time):

- **(a) exact-match set** — tüm cümlenin hash'inden oluşan bir hash set; birebir
  kopyalar beklenen O(1)'de bulunur. Bu bir `Jaccard = 1.0` **fast path'tir, bir
  "dur" değil**: bir `main_doc` cümlesi bir belgede birebir kopya *ve* başka bir
  belgede yakın kopya olabilir, ikisi de sayılmalı.
- **(b) CONTENT WORD üzerinde inverted index** —
  `contentTokenHash → (docId, sentenceId) listesi` hash map'i; token'lar **küçük bir
  stopword listesi çıkarıldıktan sonra** (~100 çok yaygın kelime: the, of, and, to,
  a …) anahtarlanır.

Her `main_doc` cümlesi için: index'ten **candidate** document cümlelerini topla
(≥1 *content* word paylaşanlar) ve full-token Jaccard'ı yalnızca candidate'lara
uygula.

> **Content word neden — "efficient"i gerçekten efficient yapan karar.**
> Örnek sette ölçüldü (2026-11-16): `"the"` tüm 2095 document cümlesinin
> **%91.3'ünde**, `"of"` %83.9'unda geçiyor. Index'i *her* token'la anahtarlamak
> "≥1 ortak token" filtresini cümlelerin **%95.4'ünü** candidate yapar → efficient
> sürüm brute force'un **0.954 katı** iş yapar (gerçek bir hızlanma yok). Stopword'leri
> **sadece index key'lerinden** çıkarmak candidate'ları **~0.12 kata** düşürür (≈8×
> az karşılaştırma) ve 8/5/4/2/1'i **yine birebir üretir** (doğrulandı). Match skoru
> hâlâ **tüm** token'ları kullandığı için accuracy değişmez — sadece shortlist küçülür.

- **Brute force ile correctness:** gerçek her yakın kopya ("küçük düzenleme") content
  word paylaşır, dolayısıyla her zaman candidate'tır; birebir kopyalar da exact-match
  set'te yakalanır. Tek teorik boşluk — iki *farklı* cümlenin threshold'a yalnızca
  **ortak stopword'lerle** ulaşması — doğal metinde makul bir threshold'da oluşmaz;
  örnek sette brute force ile eşitliği tekrar doğrular ve raporda dürüstçe belirtiriz.
- **"Hashing'i kendimiz yazdık":** token'lar ve tüm cümle için bir polynomial hash
  (Lecture 7'deki `h(K) = K mod m`'in string'e genellemesi); standard container'lar
  yalnızca hash'lenmiş key'leri saklamak için kullanılır.

Değerlendirilip **elenen alternatifler (efficient için):**
- Horspool / Boyer–Moore (Lecture 7): substring search yapar — cümle düzeyinde
  near-duplicate + similarity rate'e uymaz, worst case yine O(nm).
- Presorting (Transform-and-Conquer): yalnızca birebir kopyaları yakalar.

---

## 5. İşleme hattı (pipeline — iki sürümde ortak)

1. `main_doc.txt` ve her `documents/*.txt`'i oku.
2. **Sentence splitting:** sınır `.`, `!` veya `?`, **şu istisnalarla**: (i) iki rakam
   arasındaki nokta (`465.91` gibi decimal) ve (ii) bilinen bir abbreviation
   (`Mr. Mrs. Dr. St. Mt. No. U.S.`) ya da tek büyük harften hemen sonraki nokta.
   Sonra ardışık sınırları birleştir, trim et, boşları at. *Neden guard:* gizli
   setin çekildiği corpus bunları içeriyor — örnekte: `St.`×7, `Mr.`×4, decimal ×67.
   Guard olmadan, bunlardan birini içeren bir gizli `main_doc` (a) cümle sayısını →
   paydayı → **her** yüzdeyi değiştirir, (b) kopyalanmış bir cümleyi bölüp eşleşmez
   hale getirir. Guard sonrası örneğin yine tam **30** cümle verdiğini doğrula.
3. **Normalization:** küçük harfe çevir; her alfanumerik olmayan karakteri boşluk yap;
   boşluktan böl; tireli kelimeler ayrı token olur; rakamlar korunur.
4. Cümle başına token set kur.
5. Kurala göre eşleştir, belge başına topla, sırala, output'u formatla.

---

## 6. Complexity (rapor bölümü c)

Tanımlar: `S_m` = `main_doc` cümle sayısı; `S_d` = tüm belgelerdeki toplam cümle;
`D` = belge sayısı; `L` = cümle başına ortalama kelime.

| Adım | Brute force | Efficient (content-word index) |
|------|-------------|-------------------------------|
| Read + normalize + tokenize | O((S_m + S_d)·L) | O((S_m + S_d)·L) |
| Index kurma | — | O(S_d·L) |
| Matching | O(S_m · S_d · L) | O((S_m + toplam candidate)·L) — nota bak |
| Ekstra space | O((S_m + S_d)·L) | O(S_d·L) index için (space-for-time bedeli) |

**Efficient sınırın dürüst ifadesi.** Bir inverted-index lookup O(posting uzunluğu)
maliyetlidir. Key'ler stopword içerseydi, en sık posting'ler Θ(S_d) olur ve *typical*
durumu Θ(S_m·S_d·L)'ye — yani brute force'tan iyi değil — iter. **Content word**'le
anahtarlamak, kopyayı gerçekten gösteren ayırt edici kelimeler için posting'leri kısa
tutar; pratikte candidate toplamı S_m·S_d'nin küçük bir kesridir (örnekte ≈**0.12×**).
Biz **worst case**'i O(S_m·S_d·L) olarak (her cümlenin bir content word paylaştığı
adversarial metin) belirtir ve ölçülen tipik hızlanmayı raporlarız — garanti bir
near-linear sınır **iddia etmeyiz**. Çift başına Jaccard, `std::unordered_set` ile
**O(L)**'dir (sorting / nested loop değil); raporda bunu sabitleriz.

---

## 7. Empirical analysis (rapor bölümü d)

- Input size = belge sayısı (ve toplam cümle). **≥5 boyut**, ör. 1, 5, 10, 20 (gerçek
  farklı örnek belgeler), ~200'e doğru **corpus'tan ek farklı cümleler örnekleyerek**
  genişlet — belge **kopyalayarak değil** (kopyalama birebir aynı cümleleri enjekte
  eder; bu temsilî değildir ve brute-vs-efficient farkını her boyutta düzleştirip
  crossover'ı gizler).
- Wall-clock milisaniyeyi `std::chrono::steady_clock` ile ölç; k kez tekrarla,
  ortalamayı raporla.
- Bir CSV üret; bir **table + line plot** (brute vs efficient) çıkar.
- Beklenen şekil: brute force ~quadratic, efficient ~linear büyür; crossover
  görünür ve efficient sürümü gerekçelendirir.

---

## 8. Output formatı (`expected_output.txt` ile aynı)

```
1) document18.txt   similarity 26.7%   (8 of 30 sentences)
   "<main_doc cümlesi>"
   "<eşleşen document cümlesi>"
   ... 5 çifte kadar; blok içindeki çift sırası önemli değil
2) ...
All other documents: 0.0%.
```

Belgeler similarity'ye göre azalan sıralı. Eşleşen belgeler detaylı; sıfır
similarity'li belgeler tek satırda özetlenir (hocanın `expected_output.txt`'iyle
aynı). Yüzdeler **bir ondalığa** yuvarlanır (`8/30 = 26.666… → 26.7`). Bir belgenin
5'ten fazla eşleşen çifti varsa, en yüksek skorlu 5'i ve bir `... and N more pairs`
satırı göster. 5-çift seçimi **deterministik** yapılır — çiftleri (Jaccard azalan,
docId, sentenceId) ile sırala — böylece brute force ve efficient sürümler aynı
blokları yazar.

---

## 9. Klasör yapısı ve işbirliği

```
algoritma_analizi_proje/
├── README.md              # genel bakış · build/run · yapı · durum · ekip
├── .gitignore
├── main                   # (kod — placeholder) giriş: args, çalıştırma, timing
├── brute_force            # (kod — placeholder) detectBruteForce()
├── better_solution        # (kod — placeholder) detectEfficient() + inverted index
├── docs/
│   ├── design.md          # bu belge
│   ├── progress.md        # ilerleme günlüğü ("sürekli bilgilendirme")
│   ├── open-questions.md  # bekleyen kararlar
│   └── report/            # 4–6 sayfalık rapor + figürler
└── data/sample_set/       # verilen örnek (main_doc.txt, documents/, expected_output.txt)
```

Kod dosyaları, ekibin repoda zaten açtığı `main` / `brute_force` / `better_solution`
dosyalarıyla eşleşir; uygulama aşamasında bunlar `.cpp` olur (ve paylaşılan bir
`common.hpp` eklenir). Kök yerine bir `src/` klasörü tercih edilirse, bu ekip kararı
olarak uygulama aşamasında belirlenir.

**Standartlar**
- Repo dosyalarında **terimler English, açıklamalar Türkçe**; kod/yorum/identifier
  English.
- **Conventional Commits** (`feat:`, `fix:`, `docs:`, `test:`, `chore:`).
- Bir fonksiyon = bir sorumluluk; kısa, yorumlu fonksiyonlar.

**Sürekli bilgilendirme (herkes durumu görsün diye)**
- `docs/progress.md` her çalışma oturumunda güncellenir: ne yapıldı, sırada ne var,
  alınan kararlar.
- `README.md`'de kısa bir **status table** güncel tutulur.
- Commit mesajları açıklayıcı; her commit tek mantıksal değişiklik.

---

## 10. Kod organizasyonu (C++)

- `common` (header): tipler (`Sentence`, `Document`, `MatchPair`, `DocResult`),
  `readCorpus()`, `splitSentences()`, `normalizeTokens()`, `jaccard()`, `polyHash()`,
  `formatOutput()`.
- `brute_force`: `detectBruteForce(mainDoc, corpus, threshold)`.
- `better_solution`: `InvertedIndex`, `detectEfficient(mainDoc, corpus, threshold)`.
- `main`: argümanları parse et (varsayılan = efficient; `--brute`; `--benchmark`),
  çalıştır, yazdır, zamanla. Varsayılan çalışma, bulunulan klasörden `main_doc.txt` +
  `documents/`'ı okur ve istenen output'u yazar — gizli-set sunum çalıştırması için
  hazır.

---

## 11. Test

- **Birincil:** `data/sample_set/` üzerinde çalıştır, `expected_output.txt` ile
  karşılaştır (blok içi çift sırası ve 5-çift seçimi hariç). İki sürüm hem birbiriyle
  hem beklenen output'la uyuşmalı.
- `main_doc` tam **30** cümleye bölünüp sonuç 20 eşleşme ve doğru belge-başı sayıları
  verene kadar `THRESHOLD` ve sentence splitter'ı calibrate et.

---

## 12. Açık sorular (ekip / hoca)

1. **C++ compiler:** bu makinede compiler yok. Burada derleyip test etmek ve raporun
   timing plot'unu üretmek için bir tane kur (taşınabilir, admin gerektirmeyen, geri
   alınabilir bir seçenek var) — ya da ekip kendi makinelerinde derler. Karar gerek.
2. **"Write hashing yourselves":** kapsamı Teams task instructions / rubric ile teyit
   et. Güvenli varsayım: kendi polynomial hash'imiz + std container'lar (ödev açıkça
   "hash maps"e izin veriyor).
3. **Repo-docs dili:** varsayılan "terimler English, gerisi Türkçe". Ayrı bir tam
   English README istenir mi?

---

## 13. Deliverable → not eşlemesi

| Kriter | Puan | Nerede karşılanıyor |
|--------|------|---------------------|
| Correctness & accuracy | 25 | matching rule + örnek test vs `expected_output.txt` |
| Efficiency | 25 | hashing + content-word inverted index; timing table |
| Rapor analizi | 25 | §6 Big-O + §7 empirical (table + plot) + karşılaştırma |
| Code quality | 10 | temiz modüler C++, yorumlar, build/run talimatları |
| Presentation & Q&A | 15 | sade tasarım; her üye açıklamaya hazır |

---

## 14. Builder'a bırakılanlar (uygulama planı)

Kesin `THRESHOLD` değeri, sentence-splitter edge durumları, nihai output
formatlaması, benchmark input üretimi, plot üretimi ve rapor yazımı — hepsi uygulama
sırasında, örnek setin ground truth'una göre netleşir.
