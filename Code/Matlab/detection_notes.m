function y = detection_notes(freqs,octave)
for fi = 1:3
    if(octave==4)
        if(compare(freqs(fi),261.63,5))
            disp('Do4');
        elseif(compare(freqs(fi),277.18,10))
            disp('Do#4');
        elseif(compare(freqs(fi),293.66,10))
            disp('Re4');
        elseif(compare(freqs(fi),311.13,10))
            disp('Re#4');
        elseif(compare(freqs(fi),329.63,10))
            disp('Mi4');
        elseif(compare(freqs(fi),349.23,10))
            disp('Fa4');
        elseif(compare(freqs(fi),369.99,10))
            disp('Fa#4');
        elseif(compare(freqs(fi),392.00,10))
            disp('Sol4');
        elseif(compare(freqs(fi),415.30,10))
            disp('Sol#4');
        elseif(compare(freqs(fi),440.00,10))
            disp('La4');
        elseif(compare(freqs(fi),466.16,10))
            disp('La#4');
        elseif(compare(freqs(fi),493.88,10))
            disp('Si4');
        elseif(compare(freqs(fi),523.25,10))
            disp('Do5');
        end
    end
end
end

function y = compare(note, ref, tol)
if(abs(note-ref)<tol)
    y = 1;
    return
else
    y = 0;
    return
end
end