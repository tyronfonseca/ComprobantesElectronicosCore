import * as React from "react";
import { ChevronDown } from "lucide-react";
import { cn } from "@/lib/utils";

interface CardContextType {
  isCollapsed: boolean;
  toggleCollapse: () => void;
  setCollapsed: (collapsed: boolean) => void;
  collapsible: boolean;
}

const CardContext = React.createContext<CardContextType>({
  isCollapsed: false,
  toggleCollapse: () => {},
  setCollapsed: () => {},
  collapsible: false,
});

export const useCardContext = () => React.useContext(CardContext);

export interface CardProps extends React.HTMLAttributes<HTMLDivElement> {
  /**
   * Determina si la tarjeta puede expandirse y contraerse.
   * Por defecto es true.
   */
  collapsible?: boolean;
  /**
   * Estado inicial contraído al montar (no controlado).
   */
  defaultCollapsed?: boolean;
  /**
   * Estado contraído controlado desde el exterior.
   */
  isCollapsed?: boolean;
  /**
   * Callback invocado cuando el estado de colapso cambia.
   */
  onCollapseChange?: (collapsed: boolean) => void;
}

export const Card = React.forwardRef<HTMLDivElement, CardProps>(
  (
    {
      className,
      collapsible = true,
      defaultCollapsed = false,
      isCollapsed: controlledCollapsed,
      onCollapseChange,
      children,
      ...props
    },
    ref
  ) => {
    const [uncontrolledCollapsed, setUncontrolledCollapsed] = React.useState(defaultCollapsed);

    const isControlled = controlledCollapsed !== undefined;
    const isCollapsed = isControlled ? controlledCollapsed : uncontrolledCollapsed;

    const setCollapsed = React.useCallback(
      (value: boolean) => {
        if (!isControlled) {
          setUncontrolledCollapsed(value);
        }
        onCollapseChange?.(value);
      },
      [isControlled, onCollapseChange]
    );

    const toggleCollapse = React.useCallback(() => {
      setCollapsed(!isCollapsed);
    }, [isCollapsed, setCollapsed]);

    const contextValue = React.useMemo(
      () => ({
        isCollapsed,
        toggleCollapse,
        setCollapsed,
        collapsible,
      }),
      [isCollapsed, toggleCollapse, setCollapsed, collapsible]
    );

    return (
      <CardContext.Provider value={contextValue}>
        <div
          ref={ref}
          className={cn(
            "rounded-xl border border-slate-200 bg-white text-slate-950 shadow-sm transition-all duration-200",
            className
          )}
          {...props}
        >
          {children}
        </div>
      </CardContext.Provider>
    );
  }
);
Card.displayName = "Card";

export interface CardHeaderProps extends React.HTMLAttributes<HTMLDivElement> {
  /**
   * Oculta el botón / icono de toggle en el encabezado aun cuando la tarjeta sea colapsable.
   */
  hideToggle?: boolean;
}

export const CardHeader = React.forwardRef<HTMLDivElement, CardHeaderProps>(
  ({ className, hideToggle = false, children, onClick, onKeyDown, ...props }, ref) => {
    const { isCollapsed, toggleCollapse, collapsible } = useCardContext();

    const handleClick = (e: React.MouseEvent<HTMLDivElement>) => {
      const target = e.target as HTMLElement;
      if (
        target.closest("button, a, input, select, textarea, [role='button'], [data-prevent-collapse]")
      ) {
        onClick?.(e);
        return;
      }

      if (collapsible) {
        toggleCollapse();
      }
      onClick?.(e);
    };

    const handleKeyDown = (e: React.KeyboardEvent<HTMLDivElement>) => {
      if (collapsible && (e.key === "Enter" || e.key === " ")) {
        const target = e.target as HTMLElement;
        if (!target.closest("button, a, input, select, textarea")) {
          e.preventDefault();
          toggleCollapse();
        }
      }
      onKeyDown?.(e);
    };

    return (
      <div
        ref={ref}
        role={collapsible ? "button" : undefined}
        tabIndex={collapsible ? 0 : undefined}
        aria-expanded={collapsible ? !isCollapsed : undefined}
        onClick={handleClick}
        onKeyDown={handleKeyDown}
        className={cn(
          "flex items-start justify-between gap-4 p-6 transition-colors",
          collapsible &&
            "cursor-pointer select-none hover:bg-slate-50/50 focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-blue-500 rounded-t-xl",
          collapsible && isCollapsed && "rounded-b-xl",
          className
        )}
        {...props}
      >
        <div className="flex flex-col space-y-1.5 flex-1 min-w-0">{children}</div>
        {collapsible && !hideToggle && (
          <button
            type="button"
            aria-label={isCollapsed ? "Expandir sección" : "Contraer sección"}
            className="text-slate-400 hover:text-slate-700 hover:bg-slate-100 rounded-lg p-1.5 transition-all focus:outline-none focus:ring-2 focus:ring-blue-500 shrink-0 mt-0.5"
            onClick={(e) => {
              e.stopPropagation();
              toggleCollapse();
            }}
          >
            <ChevronDown
              className={cn(
                "w-4 h-4 transition-transform duration-200",
                isCollapsed ? "-rotate-90 text-slate-400" : "rotate-0 text-slate-600"
              )}
            />
          </button>
        )}
      </div>
    );
  }
);
CardHeader.displayName = "CardHeader";

export const CardToggle = React.forwardRef<
  HTMLButtonElement,
  React.ButtonHTMLAttributes<HTMLButtonElement>
>(({ className, ...props }, ref) => {
  const { isCollapsed, toggleCollapse, collapsible } = useCardContext();

  if (!collapsible) return null;

  return (
    <button
      ref={ref}
      type="button"
      onClick={(e) => {
        e.stopPropagation();
        toggleCollapse();
      }}
      className={cn(
        "text-slate-400 hover:text-slate-700 hover:bg-slate-100 rounded-lg p-1.5 transition-all focus:outline-none focus:ring-2 focus:ring-blue-500",
        className
      )}
      aria-label={isCollapsed ? "Expandir sección" : "Contraer sección"}
      {...props}
    >
      <ChevronDown
        className={cn(
          "w-4 h-4 transition-transform duration-200",
          isCollapsed ? "-rotate-90 text-slate-400" : "rotate-0 text-slate-600"
        )}
      />
    </button>
  );
});
CardToggle.displayName = "CardToggle";

export const CardTitle = React.forwardRef<HTMLHeadingElement, React.HTMLAttributes<HTMLHeadingElement>>(
  ({ className, ...props }, ref) => (
    <h3 ref={ref} className={cn("font-semibold leading-none tracking-tight text-slate-900", className)} {...props} />
  )
);
CardTitle.displayName = "CardTitle";

export const CardDescription = React.forwardRef<HTMLParagraphElement, React.HTMLAttributes<HTMLParagraphElement>>(
  ({ className, ...props }, ref) => (
    <p ref={ref} className={cn("text-sm text-slate-500", className)} {...props} />
  )
);
CardDescription.displayName = "CardDescription";

export const CardContent = React.forwardRef<HTMLDivElement, React.HTMLAttributes<HTMLDivElement>>(
  ({ className, ...props }, ref) => {
    const { isCollapsed, collapsible } = useCardContext();

    if (collapsible && isCollapsed) {
      return null;
    }

    return <div ref={ref} className={cn("p-6 pt-0 animate-in fade-in-50 duration-150", className)} {...props} />;
  }
);
CardContent.displayName = "CardContent";

export const CardFooter = React.forwardRef<HTMLDivElement, React.HTMLAttributes<HTMLDivElement>>(
  ({ className, ...props }, ref) => {
    const { isCollapsed, collapsible } = useCardContext();

    if (collapsible && isCollapsed) {
      return null;
    }

    return (
      <div
        ref={ref}
        className={cn("flex items-center p-6 pt-0 animate-in fade-in-50 duration-150", className)}
        {...props}
      />
    );
  }
);
CardFooter.displayName = "CardFooter";
