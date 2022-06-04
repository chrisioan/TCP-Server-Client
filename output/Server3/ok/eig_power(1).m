function [lambda,znew] = eig_power(A,y,tol,maxiter)
% ��������� ��� ��� ���������� ��� ����������� ���� ������� ���� ��������� ��� ��� �����������
% ��������������� ���� ������ � (������� ��� ��������)

lambda0=0;

%������ �������� p ������� ���� |yp|=max||yi||, i=1,...,n
p=min(find (abs(y)==max(abs(y))));  % ������� �� ���� ji
 disp('p'); disp(p);
pp=norm(y, Inf);% ������� �� ���������� ����. ��� ���������� ��� �� ���������� ��� �������
 disp('pp'); disp(pp);


z=y/y(p);
 disp('z'); disp(z);

znew=z;
    for i=1:maxiter
               ynew=A*znew; %���� 5.1
                    disp('ynew'); disp(ynew);
               pp=norm(ynew, Inf);% ������� �� ���������� ����. ��� ���������� ��� �� ���������� ��� �������
                    disp('pp'); disp(pp);
               lambda=ynew(p); %���� 5.2
                    disp('lambda'); disp(lambda);
               p=min(find (abs(ynew)==max(abs(ynew))));  % ���� 5.4
                    disp('p'); disp(p);
               znew=ynew/ynew(p); %���� 5.5
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
        s=sprintf('��� �������� ���� ��� %d, �����������', maxiter);
        disp(s);
     end; 



