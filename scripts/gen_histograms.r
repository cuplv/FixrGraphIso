groum <- read.table(file("pattern_groum"), header = TRUE)
png(filename="grouminer_patterns_hist.png")
groumplot <- hist(groum$size, main="Distribution size for mined patterns",  xlab="GrouMiner Patterns",  border="blue")
groumplot


iso <- read.table(file("pattern_iso"), header = TRUE)
png(filename="graphiso_patterns_hist.png")
isoplot <- hist(iso$size, main="Distribution size for mined patterns",  xlab="GraphIso Patterns",  border="blue")
isoplot

quit