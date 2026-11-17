# Açık Sorular & Bekleyen Kararlar

Uygulamaya başlamadan önce / uygulama sırasında ekipten gereken kararlar. Çözülenler
en alta taşınır. (Tasarımın kendisi `design.md`'de; burası sadece teyit edilecekler
listesi.)

## Bekleyen

### 1. Tasarım onayı
`docs/design.md` onaylanıp (örnek sette doğrulandı, inceleme düzeltmeleri uygulandı)
adım adım uygulama planına ve C++ koduna geçilsin mi?
**Öneri:** onayla.

### 2. Development makinesinde C++ compiler
Bu makinede şu an **hiç C++ compiler yok** (g++, clang veya MSVC yok).
- **(a)** Buraya taşınabilir, admin gerektirmeyen, geri alınabilir bir toolchain kur
  (w64devkit = g++ + make; silmek için klasörü silmek yeterli) → kodu burada derler,
  `expected_output.txt`'e karşı test eder ve raporun timing table + plot'unu üretiriz.
  _(önerilen)_
- **(b)** Kurma — kodu yaz ve matching mantığını bir Node reference oracle ile
  doğrula; ekip kendi makinelerinde derleyip timing toplar.

### 3. "Write the core hashing yourselves" — kesin kapsam
El yazması bir hash **function**'ının (polynomial hash) standard
`unordered_map/unordered_set` ile kullanılmasının yeterli olup olmadığını Teams task
instructions / rubric'ten teyit et. Ödev metni açıkça "hash maps"e izin veriyor.
**Güvenli varsayım:** kendi polynomial hash'imiz + std container'lar.

### 4. Repo-docs dili
Kod ve rapor **English** olmalı (akademik teslim). Dokümanlar şu an "terimler English,
gerisi Türkçe". Ayrıca tam **English** bir README sürümü de istenir mi?
**Varsayılan:** terimler English + Türkçe açıklama.

### 5. Ekip listesi
3–4 üyenin isim + öğrenci numaraları — README için ve teslim PDF'inin adı için gerekli
(her üyenin öğrenci numarasıyla adlandırılır).

## Çözülenler

_(henüz yok)_
