function [ ] = plotHistogram( inPath, outPath, plotTitle )

histo = dlmread(inPath);

figure
bar(0:255, histo, 1);
xlim([-3, 258]);
title(plotTitle);
saveas(gcf, outPath);
close
end

