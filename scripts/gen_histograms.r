groum <- read.table(file("pattern_groum"), header = TRUE)
png(filename="grouminer_patterns_hist.png")
groumplot <- hist(groum$freq, main="Distribution frequency for mined patterns",  xlab="GrouMiner Pattern Frequency", ylab="Number of patterns", border="blue")
groumplot

iso <- read.table(file("pattern_iso"), header = TRUE)
png(filename="graphiso_patterns_hist.png")
isoplot <- hist(iso$freq, main="Distribution frequency for mined patterns",  xlab="GraphIso Pattern Frequency", ylab="Number of patterns", border="blue")
isoplot


groum_01 <- read.table(file("01_pattern_groum"), header = TRUE)
png(filename="01_grouminer_patterns_hist.png")
groumplot <- hist(groum_01$freq, main="Distribution frequency for mined patterns",  xlab="GrouMiner Pattern Frequency", ylab="Number of patterns", border="blue")
groumplot

iso_01 <- read.table(file("01_pattern_iso"), header = TRUE)
png(filename="01_graphiso_patterns_hist.png")
isoplot <- hist(iso_01$freq, main="Distribution frequency for mined patterns",  xlab="GraphIso Pattern Frequency", ylab="Number of patterns", border="blue")
isoplot

quit