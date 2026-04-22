== Άσκηση 2: Αύξηση Κοινόχρηστης Μεταβλητής

#image("res/img/increment_compare.png", width: 100%, height: 25%)

Πολλαπλά νήματα αυξάνουν μία κοινόχρηστη μεταβλητή. Υλοποιήθηκαν τρεις μηχανισμοί συγχρονισμού: mutex, rwlock και atomic operations σε write-only workload. Τα benchmarks εκτελέστηκαν με 2, 4 και 8 threads για $N in {10^3, 10^4, 10^5, 10^6, 10^7}$ iterations.

=== Ανάλυση

*Atomic operations: Σαφής νικητής.* Τα atomics υπερέχουν σε όλα τα μεγέθη workload και thread counts. Για $N=10^7$ με 8 threads: atomics 0.181s vs mutex 0.830s vs rwlock 1.498s. Τα atomics είναι *4.6× ταχύτερα* από το mutex και *8.3× ταχύτερα* από το rwlock. Η υπεροχή είναι ακόμη μεγαλύτερη σε μικρά workloads ($N=10^3$: atomics 0.00024s vs mutex 0.00040s = 1.67× ταχύτερα).

*Thread scalability - Atomics:* Τα atomics κλιμακώνονται εξαιρετικά. Για $N=10^7$: 2t=0.180s, 4t=0.178s, 8t=0.181s - σχεδόν μηδενική υποβάθμιση με αύξηση threads. Επιτυγχάνουν ~55 Mops/sec ανεξαρτήτως thread count, αποδεικνύοντας ελάχιστο cache coherence overhead.

*Thread scalability - Mutex:* Το mutex υποβαθμίζεται σοβαρά με περισσότερα threads. Για $N=10^7$: 2t=0.386s → 4t=0.397s → 8t=0.830s. Με 8 threads είναι *2.1× πιο αργό* από 2 threads, λόγω kernel context switches και lock contention. Για μικρά workloads ($N=10^3$) είναι ανταγωνιστικό (0.30ms με 2t), αλλά καταρρέει με περισσότερα threads (0.40ms με 8t).

*Thread scalability - Rwlock:* Το rwlock έχει τη χειρότερη κλιμάκωση. Για $N=10^7$: 2t=0.869s → 4t=1.010s → 8t=1.498s. Με 8 threads είναι *1.72× πιο αργό* από 2 threads. Σε μικρότερα workloads η υποβάθμιση είναι ακόμη χειρότερη ($N=10^5$: 2t=8.8ms → 8t=16.6ms = 1.89× πιο αργό).

*Αιτίες υπεροχής atomics:*
- Lock-free: μηδενικό kernel overhead, χωρίς context switches
- Hardware-level synchronization: LOCK prefix στο x86 αντί για syscalls
- Minimal cache footprint: single cache line vs πολύπλοκες lock structures
- Αποφυγή priority inversion και scheduler overhead

*Γιατί το rwlock αποτυγχάνει:*
- Write-only workload → μηδενικός παραλληλισμός (όλα τα threads αποκλείονται)
- Πολύπλοκη εσωτερική δομή (reader/writer queues) χωρίς όφελος
- Overhead 4-6 atomic operations ανά lock/unlock vs 2 για mutex
- Μεγαλύτερη δομή → περισσότερο cache coherence traffic

=== Συμπέρασμα

Για write-only workloads: *atomics >> mutex >> rwlock*. Τα atomic operations είναι η βέλτιστη επιλογή σε όλα τα σενάρια, προσφέροντας 4-8× καλύτερη απόδοση και εξαιρετική scalability. Το mutex υποβαθμίζεται με αύξηση threads αλλά παραμένει χρησιμοποιήσιμο. Το rwlock δεν δικαιολογείται ποτέ σε write-heavy scenarios - σχεδιάστηκε αποκλειστικά για read-heavy workloads με concurrent readers.

