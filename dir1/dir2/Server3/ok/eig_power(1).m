function [lambda,znew] = eig_power(A,y,tol,maxiter)
% Συνάρτηση για την προσέγγιση της μεγαλύτερης κατ΄ απόλυτη τιμή ιδιοτιμής και του αντίστοιχου
% ιδιοδιανύσματος ενός πίνακα Α (μέθοδος των δυνάμεων)

lambda0=0;

%Εύρεση ακέραιου p τέτοιου ώστε |yp|=max||yi||, i=1,...,n
p=min(find (abs(y)==max(abs(y))));  % Βρίσκει τη θέση ji
 disp('p'); disp(p);
pp=norm(y, Inf);% Βρίσκει τη μεγαλύτερη τιμή. Δεν χρειάζεται για το αποτέλεσμα της μεθόδου
 disp('pp'); disp(pp);


z=y/y(p);
 disp('z'); disp(z);

znew=z;
    for i=1:maxiter
               ynew=A*znew; %Βήμα 5.1
                    disp('ynew'); disp(ynew);
               pp=norm(ynew, Inf);% Βρίσκει τη μεγαλύτερη τιμή. Δεν χρειάζεται για το αποτέλεσμα της μεθόδου
                    disp('pp'); disp(pp);
               lambda=ynew(p); %Βήμα 5.2
                    disp('lambda'); disp(lambda);
               p=min(find (abs(ynew)==max(abs(ynew))));  % Βήμα 5.4
                    disp('p'); disp(p);
               znew=ynew/ynew(p); %Βήμα 5.5
                    disp('znew');disp(znew);
               if abs(lambda0-lambda)<tol
                   disp('lambda');disp(lambda);
                   disp('z(znew)');disp(znew);
                   return;
               end;
               lambda0=lambda;
                   disp('lambda0'); disp(lambda0);       
    end;%for
     if abs(lambda0-lambda)>=tol
        s=sprintf('Όχι σύγκλιση μετά από %d, επαναλήψεις', maxiter);
        disp(s);
     end; 



