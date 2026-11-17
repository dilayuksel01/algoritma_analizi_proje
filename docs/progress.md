# İlerleme Günlüğü (Progress Log)

Her işbirlikçinin projenin nerede olduğunu görebilmesi için çalışan bir kayıt. En yeni
giriş en üstte. Her giriş: ne yapıldı, sırada ne var, kararlar, açık sorular.

---

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
