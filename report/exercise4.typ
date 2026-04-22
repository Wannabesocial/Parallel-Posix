== Άσκηση 4: Τραπεζικές Συναλλαγές

#image("res/img/transactions_results_time.png", width: 100%)

#table(
  columns: (auto, auto, auto, auto, auto),
  align: center,
  [*Προσέγγιση*], [*0% Query*], [*25% Query*], [*50% Query*], [*90% Query*],
  [Coarse Mutex], [159.4 ms], [158.5 ms], [159.2 ms], [159.3 ms],
  [Fine Mutex], [13.8 ms], [15.2 ms], [14.3 ms], [13.3 ms],
  [Coarse RWLock], [149.7 ms], [150.0 ms], [149.3 ms], [149.8 ms],
  [Fine RWLock], [16.4 ms], [15.5 ms], [16.1 ms], [16.5 ms],
)

_Παράμετροι: 10K accounts, 8 threads, 75K συναλλαγές_

Προσομοίωση τράπεζας με transfers (write) και queries (read) σε κοινόχρηστο πίνακα λογαριασμών. Υλοποιήθηκαν 4 σχήματα συγχρονισμού: coarse/fine-grained με mutex/rwlock.

=== Coarse vs Fine-Grained

Το fine-grained είναι $11.5$x ταχύτερο από το coarse-grained (13.8 ms vs 159.4 ms για 0% queries). Με 10K accounts και τυχαίες συναλλαγές, η πιθανότητα δύο threads να χρειαστούν τον ίδιο λογαριασμό είναι ~0.08%. Το coarse-grained κλειδώνει όλους τους λογαριασμούς σε κάθε συναλλαγή, ενώ το fine-grained κλειδώνει μόνο τους 2 που χρειάζεται. Έτσι στο coarse-grained τα threads εκτελούνται σειριακά, ενώ στο fine-grained τρέχουν παράλληλα χωρίς contention.

#table(
  columns: (auto, auto, auto, auto),
  align: center,
  [*Accounts*], [*Coarse Mutex*], [*Fine Mutex*], [*Speedup*],
  [100], [3.5 ms], [0.7 ms], [5.0×],
  [1,000], [15.7 ms], [3.1 ms], [5.1×],
  [10,000], [159.3 ms], [13.8 ms], [11.5×],
)

Το όφελος του fine-grained αυξάνεται με περισσότερα accounts ($5.0$x $arrow.r$ $11.5$x) διότι μειώνεται η πιθανότητα contention (10% για 100 accounts $arrow.r$ 0.08% για 10K accounts).

=== Mutex vs RWLock

Στο fine-grained το mutex είναι ελαφρώς ταχύτερο (13.3-15.2 ms vs 15.5-16.5 ms, ~10-15% βελτίωση). Στο coarse-grained το rwlock είναι καλύτερο (149.7 ms vs 159.4 ms, 6% βελτίωση).

Το rwlock επιτρέπει concurrent reads. Στο coarse-grained που ο παραλληλισμός είναι μηδενικός, τα παράλληλα queries παρέχουν βελτίωση. Στο fine-grained που ήδη υπάρχει υψηλός παραλληλισμός, το rwlock overhead (4-6 atomic ops vs 2 για mutex) είναι μεγαλύτερο από το όφελος. Επίσης τα queries είναι πολύ γρήγορα (απλό array read), οπότε το rwlock overhead δεν αντισταθμίζεται.

=== Επίδραση Query Percentage

Στο fine-grained mutex παρατηρείται μικρή διακύμανση (13.3-15.2 ms, ~10-15%) ανεξαρτήτως query percentage. Το fine-grained ήδη επιτρέπει μέγιστο παραλληλισμό.

Στο coarse-grained mutex δεν υπάρχει επίδραση (~159 ms σε όλα τα percentages) διότι το mutex δεν επιτρέπει concurrent reads.

Στο coarse-grained rwlock η επίδραση είναι minimal (~0.1%) επειδή το query critical section είναι τόσο σύντομο που το rwlock overhead ισοσκελίζει το concurrent read benefit.

=== Πείραμα με usleep

Για να επιβεβαιωθεί ότι το rwlock υπερτερεί σε μεγάλα critical sections, προστέθηκε `usleep(10)` μέσα στο query operation. Με μεγαλύτερο query critical section, τα concurrent reads του rwlock υπερκαλύπτουν το overhead και το fine rwlock ξεπερνά το fine mutex.

Άρα το rwlock είναι ωφέλιμο μόνο όταν:
1. Το read/write ratio είναι υψηλό (>75% reads)
2. Το read critical section είναι μεγάλο (>10μs)
3. Υπάρχει υψηλή contention (coarse-grained ή λίγα accounts)

=== Συμπέρασμα

Το fine-grained mutex είναι η βέλτιστη επιλογή: $11.5$x ταχύτερο από coarse-grained, απλούστερο από rwlock, και ανταγωνιστικό σε όλα τα query percentages.

Το granularity (fine vs coarse) είναι πολύ πιο σημαντικό από το synchronization mechanism (mutex vs rwlock). Το fine-grained με απλό mutex υπερτερεί του coarse-grained με rwlock.
